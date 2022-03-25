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

namespace {

class WindowsDarkModePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  WindowsDarkModePlugin();

  virtual ~WindowsDarkModePlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  std::string  WindowsDarkModePlugin::getPlatformVersion();
  bool WindowsDarkModePlugin::isDarkModeAppEnabled();
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void WindowsDarkModePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "windows_dark_mode",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<WindowsDarkModePlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

WindowsDarkModePlugin::WindowsDarkModePlugin() {}

WindowsDarkModePlugin::~WindowsDarkModePlugin() {}

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
