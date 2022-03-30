import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:windows_dark_mode/windows_dark_mode.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  bool _darkModeEnabled=false;
  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion =
          await WindowsDarkMode.platformVersion ?? 'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }
    _darkModeEnabled=await WindowsDarkMode.isDarkModeAppEnabled;


    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {
    return StreamBuilder<ThemeMode>(
        stream: WindowsDarkMode.DarkModeStream(),
        builder: (context, snapshot) {
          return MaterialApp(
            themeMode: snapshot.data,
            theme: ThemeData(
              brightness: Brightness.light,
            ),
            darkTheme: ThemeData(
              brightness: Brightness.dark,
            ),
            home: Scaffold(
              appBar: AppBar(
                title: const Text('Plugin example app'),
              ),
              body: Center(
                child: Text(
                    'Running on: $_platformVersion'),
              ),
            ),
          );
        });
  }
}
