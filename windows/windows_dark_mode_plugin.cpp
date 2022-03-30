#include "include/windows_dark_mode/windows_dark_mode_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <flutter/event_channel.h>
#include <mutex>

namespace {


template<typename T = flutter::EncodableValue>
class MyStreamHandler: public flutter::StreamHandler<T> {
public:
	MyStreamHandler () = default;
	virtual ~MyStreamHandler () = default;

	void on_callback (flutter::EncodableValue _data) {
            std::unique_lock<std::mutex> _ul (m_mtx);
            if (m_sink.get ())
    		    m_sink.get ()->Success (_data);
    	}
protected:
	std::unique_ptr<flutter::StreamHandlerError<T>> OnListenInternal (const T *arguments, std::unique_ptr<flutter::EventSink<T>> &&events) override {
        std::unique_lock<std::mutex> _ul (m_mtx);
		m_sink = std::move (events);
        return nullptr;
	}
	std::unique_ptr<flutter::StreamHandlerError<T>> OnCancelInternal (const T *arguments) override {
        std::unique_lock<std::mutex> _ul (m_mtx);
		m_sink.release ();
        return nullptr;
	}
private:
	flutter::EncodableValue m_value;
    std::mutex m_mtx;
	std::unique_ptr<flutter::EventSink<T>> m_sink;
};


class WindowsDarkModePlugin : public flutter::Plugin {
 public:
  // This method will register the Plugin with Flutter Engine
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
  // Constructor
  WindowsDarkModePlugin(flutter::PluginRegistrarWindows *registrar);
  // Destructor
  virtual ~WindowsDarkModePlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
   void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  // Specialized method called from HandleMethodCall
  std::string  WindowsDarkModePlugin::getPlatformVersion();
  // Specialized method called from HandleMethodCall
  bool WindowsDarkModePlugin::isDarkModeAppEnabled();
  // Our WndProc Callback
  std::optional<LRESULT> HandleWindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
  
  int window_proc_id = -1;
  MyStreamHandler<> *m_handler;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> m_event_channel;
  flutter::PluginRegistrarWindows* registrar;
};

// static
void WindowsDarkModePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "windows_dark_mode",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<WindowsDarkModePlugin>(registrar);

plugin->m_event_channel = std::make_unique<flutter::EventChannel<flutter::EncodableValue>> (
          registrar->messenger (), "windows_dark_mode/dark_mode_callback",
          &flutter::StandardMethodCodec::GetInstance ()
       );
  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });


MyStreamHandler<> *_handler=new MyStreamHandler<> ();
     plugin->m_handler = _handler;
     auto _obj_stm_handle = static_cast<flutter::StreamHandler<flutter::EncodableValue>*> (plugin->m_handler);
     std::unique_ptr<flutter::StreamHandler<flutter::EncodableValue>> _ptr {_obj_stm_handle};
     plugin->m_event_channel->SetStreamHandler (std::move (_ptr));
	 
	 
  registrar->AddPlugin(std::move(plugin));
}

std::optional<LRESULT> WindowsDarkModePlugin::HandleWindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
 std::optional<LRESULT> result = std::nullopt;
  if (message == WM_SETTINGCHANGE)
             {
                if (!lstrcmp(LPCTSTR(lParam), L"ImmersiveColorSet"))
                        {
                           if (m_handler!=nullptr) {
                            m_handler->on_callback (flutter::EncodableValue(isDarkModeAppEnabled()));
                            return true;
                           }
                        }
             }
 return result;
}

WindowsDarkModePlugin::WindowsDarkModePlugin(flutter::PluginRegistrarWindows *registrar) {
   this->registrar=registrar;
   window_proc_id = registrar->RegisterTopLevelWindowProcDelegate(
        [this](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
          return HandleWindowProc(hWnd, message, wParam, lParam);
        });
}

WindowsDarkModePlugin::~WindowsDarkModePlugin() {
  registrar->UnregisterTopLevelWindowProcDelegate(window_proc_id);
}

std::string  WindowsDarkModePlugin::getPlatformVersion(){
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    return version_stream.str();
}

bool WindowsDarkModePlugin::isDarkModeAppEnabled() {
     auto buffer = std::vector<char>(4);
    auto cbData = static_cast<DWORD>(buffer.size() * sizeof(char));
    auto res = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD, // expected value type
        nullptr,
        buffer.data(),
        &cbData);

    if (res != ERROR_SUCCESS) {
        throw std::runtime_error("Error: error_code=" + std::to_string(res));
    }
    // convert bytes written to our buffer to an int, assuming little-endian
    auto i = int(buffer[3] << 24 |
        buffer[2] << 16 |
        buffer[1] << 8 |
        buffer[0]);
    return i != 1;
}

void WindowsDarkModePlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {

  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    result->Success(flutter::EncodableValue(WindowsDarkModePlugin::getPlatformVersion()));
    return;  }

  if (method_call.method_name().compare("isDarkModeAppEnabled") == 0) {
    result->Success(flutter::EncodableValue(WindowsDarkModePlugin::isDarkModeAppEnabled()));
    return;  }

  result->NotImplemented();
}

}  // namespace

void WindowsDarkModePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WindowsDarkModePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
