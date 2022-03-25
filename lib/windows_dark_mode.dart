
import 'dart:async';

import 'package:flutter/services.dart';

class WindowsDarkMode {
  static const MethodChannel _channel = MethodChannel('windows_dark_mode');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
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
