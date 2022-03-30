
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class WindowsDarkMode {
  static const MethodChannel _channel = MethodChannel('windows_dark_mode');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

static const _eventChannel =
  EventChannel('windows_dark_mode/dark_mode_callback');

 static Stream<ThemeMode> DarkModeStream() {
    return _eventChannel
        .receiveBroadcastStream()
        .map((darkMode) {
      return darkMode == true ? ThemeMode.dark : ThemeMode.light;
    })
    .distinct();
  }

  static Future<bool> get isDarkModeAppEnabled async {
    try {
      return await _channel.invokeMethod('isDarkModeAppEnabled');
    }
    catch  (_){
      return false;
    }
  }
}
