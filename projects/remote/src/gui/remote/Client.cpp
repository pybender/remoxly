#include <vector>
#include <sstream>
#include <gui/remote/Serializer.h>
#include <gui/remote/Client.h>

// -----------------------------------------------------------

int remoxly_client_websocket(struct libwebsocket_context* ctx,
                             struct libwebsocket* ws,
                             enum libwebsocket_callback_reasons reason,
                             void* user,
                             void* in, 
                             size_t len)
{

  Client* client = static_cast<Client*>(libwebsocket_context_user(ctx));

  switch(reason) {

    case LWS_CALLBACK_CLIENT_ESTABLISHED: {
      client->state = REMOTE_STATE_CONNECTED;

      if(!client->isApplication()) {
        client->createGetGuiModelTask();
      }
      else {
        client->createSetGuiModelTask();
      }

      libwebsocket_callback_on_writable(ctx, ws);
      break;
    }

    case LWS_CALLBACK_CLIENT_WRITEABLE: {
      return client->onCallbackClientWritable();
    }

    case LWS_CALLBACK_CLIENT_RECEIVE: {
      return client->onCallbackReceive((char*)in, len);
    }

    // reconnect when connection is closed.
    case LWS_CALLBACK_CLOSED:
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
      
      client->state = REMOTE_STATE_DISCONNECTED;
      client->reconnect_timeout = remoxly_hrtime() + client->reconnect_delay;
      client->onDisconnected();
      break;
    }

    default: {
      printf("Unhandled reason: %s\n", remoxly_websocket_reason_to_string(reason).c_str());
      break;
    }
  }
  return 0;
}

// -----------------------------------------------------------

Client::Client(std::string host, int port, bool ssl, ClientListener* listener) 
  :host(host)
  ,port(port)
  ,use_ssl(ssl)
  ,context(NULL)
  ,ws(NULL)
  ,listener(listener)
  ,state(REMOTE_STATE_NONE)
  ,reconnect_timeout(0)
  ,reconnect_delay(1000ULL * 1000000ULL)  
  ,is_application(false)
{

  // setup the creation info.
  memset(&info, 0, sizeof info);

  info.port = CONTEXT_PORT_NO_LISTEN;
  info.gid = -1;
  info.uid = -1;
  info.protocols = remoxly_client_protocol;
  info.user = (void*)this;

#ifndef LWS_NO_EXTENSIONS
  info.extensions = libwebsocket_get_internal_extensions();
#endif

}

Client::~Client() {
  shutdown();
}

void Client::shutdown() {

  if(context) {
    libwebsocket_context_destroy(context);
    context = NULL;
  }

  for(std::vector<ConnectionTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
    delete *it;
  }

  tasks.clear();
}

bool Client::connect() {
  
  // when the serializer can serialize at this moment it means the client is used for an application.
  is_application = serializer.canSerialize(); 

  if(!createContext()) {
    return false;
  }
  
  if(!createConnection()) {
    shutdown();
    return false;
  }

  /*
  if(!createSetGuiModelTask()) {
    return false;
  }
  if(!createGetGuiModelTask()) {
    return false;
  }
  */

  return true;
}

void Client::update() {

#if !defined(NDEBUG)
  if(!context) {
    printf("Error: no valid client websocket context found. Cannot update().\n");
    ::exit(EXIT_FAILURE);
  }
#endif
  
  // reconnect when not connected.
  if(state == REMOTE_STATE_DISCONNECTED) {

    uint64_t now = remoxly_hrtime();

    if(now < reconnect_timeout) {
      return;
    }

    if(!createConnection()) {
      printf("Warning: Cannot reconnect.\n");
    }
  }

  int n = libwebsocket_service(context, 0);
}

void Client::addPanel(Panel* panel) {
  serializer.addPanel(panel);
  setWidgets(panel);
  panel->addListener(this);
}

void Client::addGroup(Group* group) {
  setWidgets(group);
  serializer.addGroup(group);
  group->addListener(this);
}

void Client::setWidgets(Panel* panel) {
  for(std::vector<Group*>::iterator it = panel->groups.begin(); it != panel->groups.end(); ++it) {
    setWidgets((*it));
  }
}

void Client::setWidgets(Group* group) {
  for(std::vector<Widget*>::iterator it = group->children.begin(); it != group->children.end(); ++it) {
    Widget* w = *it;
    widgets[w->id] = w;
  }
}

bool Client::createContext() {

  if(context) {
    printf("Errro: already have a context.\n");
    return false;
  }

  context = libwebsocket_create_context(&info);

  if(context == NULL) {
    printf("Error: cannot create the client websocket context.\n");
    return false;
  }

  return true;
}

bool Client::createConnection() {

  if(state == REMOTE_STATE_CONNECTING) {
    printf("Error: already trying to connect.\n");
    return false;
  }

  state = REMOTE_STATE_CONNECTING;

  // @todo -> not sure about the host/origin parameters!
  ws = libwebsocket_client_connect(context,
                                   host.c_str(),
                                   port, 
                                   use_ssl,
                                   "/",
                                   host.c_str(),
                                   host.c_str(),
                                   remoxly_client_protocol[0].name, 
                                   -1);

  if(!ws) {
    printf("Error: cannot create client connection.\n");
    return false;
  }

  return true;
}

bool Client::addTask(int taskID, int appID, std::string value) {

  if(!ws) {
    printf("Error: no websocket connection found.\n");
    return false;
  }

  if(!context) {
    printf("Error: no valid websocket context found.\n");
    return false;
  }


  ConnectionTask* task = new ConnectionTask();
  task->task_name = taskID;
  task->task_id = appID;
  task->task_data = value;

  tasks.push_back(task);

  libwebsocket_callback_on_writable(context, ws);
  return true;
}

bool Client::createGetGuiModelTask() {

  if(serializer.canSerialize()) {
    return true;
  }

  addTask(REMOTE_TASK_GET_GUI_MODEL, 0);

  return true;
}

// will only try to create a gui model when the user added panels/groups, else it will just return true
bool Client::createSetGuiModelTask() {
  
  // does the serialize has panels/groups?
  if(!serializer.canSerialize()) {
    return true; 
  }

  // serialize when there have been panels/group added
  // it's okay when they havent been added; we will just
  // wait untill the server sends one to us based on the 
  // connection/gui id.
  std::string gui_model;
  if(!serializer.serialize(gui_model))  {
    printf("Error: cannot serialize the gui.\n");
    return false;      
  }

  return addTask(REMOTE_TASK_SET_GUI_MODEL, 0, gui_model);
}

int Client::onCallbackClientWritable() {

  int result = 0;

  for(size_t i = 0; i < tasks.size(); ++i) {
          
    ConnectionTask* task = tasks[i];

    switch(task->task_name) {

      case REMOTE_TASK_GET_GUI_MODEL:
      case REMOTE_TASK_SET_GUI_MODEL: {
        sendTask(task);
        break;
      }

      case REMOTE_TASK_VALUE_CHANGED: {
        sendTask(task);
        break;
      }

      default: {
        printf("Warning: cannot handle client task: %d\n", task->task_name);
        break;
      }
    }

    delete task;
    task = NULL;
  }

  tasks.clear();

  return 0;
}

void Client::onDisconnected() {

  if(listener) {
    listener->onDisconnected();
  }
  
  if(!isApplication()) {
    widgets.clear();
    serializer.clear();
  }

  if(tasks.size()) {
    printf("WARNING - WE STILL HAVE SOME TASKS (?)\n");
  }
}


// an "application" calls addGroup/addPanel (which are internally added to the serializer). 
bool Client::isApplication() {
  return is_application;
}

bool Client::onTaskValueChanged(char* data, size_t len, std::string value) {

  json_error_t err;
  json_t* js_widget = json_loads(value.c_str(), 0, &err);

  if(!js_widget) {
    printf("Error: cannot parse value json.\n");
    REMOXLY_FREE_JSON(js_widget);
    return false;
  }

  int id = 0;
  if(!remoxly_json_get_int(js_widget, "i", id)) {
    printf("Error: cannot find id from the value changed task.\n");
    REMOXLY_FREE_JSON(js_widget);
    return false;
  }

  std::map<int, Widget*>::iterator it = widgets.find(id);
  if(it != widgets.end()) {
    deserializer.deserializeChangedValue(it->second, value);
  }

  REMOXLY_FREE_JSON(js_widget);

  return true;
}

int Client::onCallbackReceive(char* data, size_t len) {

  if(!data) {
    printf("Error: cannot parse incoming data; invalid data.\n");
    return -1;
  }

  if(!len) {
    printf("Error: cannot parse incoming data; invalid size: %ld.\n", len);
    return -1;
  }

  int task_id = 0;
  int app_id = 0;
  std::string value;
  
  if(!deserializer.deserializeTask(data, app_id, task_id, value)) {
    return -1;
  }

  switch(task_id) {

    case REMOTE_TASK_SET_GUI_MODEL: {
      if(listener) {
        listener->onTaskSetGuiModel(data, len, value);
      }
      break;
    }

    case REMOTE_TASK_VALUE_CHANGED: {
      if(!onTaskValueChanged(data, len, value)) {
        return -1;
      }
      break;
    }

    default: {
      printf("Warning: unhandled server task: %d\n", task_id);
      break;
    }
  }
  printf("Task: %d, App: %d\n", task_id, app_id);
 
  return 0;
}

bool Client::sendTask(ConnectionTask* task) {

  if(!task) {
    printf("Error: cannot send the task; it's an invalid pointer.\n");
    return false;
  }

  std::string js_task = serializer.serializeTask(task->task_name, 
                                                 task->task_data, 
                                                 task->task_id);

  buffer.set(js_task);

  int r = remoxly_websocket_write(ws, buffer.ptr(), buffer.getDataNumBytes());

  return r == 0;
}

void Client::onEvent(int event, Widget* w) {

  if(event != GUI_EVENT_VALUE_CHANGED) {
    return;

  }

  std::string widget_json;

  if(!serializer.serializeChangedValue(w, widget_json)) {
    return;
  }

  ConnectionTask* task = new ConnectionTask();
  task->task_name = REMOTE_TASK_VALUE_CHANGED;
  task->task_id = 0; // connection/gui, @todo fix
  task->task_data = widget_json;
  tasks.push_back(task);

  // trigger a write
  libwebsocket_callback_on_writable(context, ws);
}
