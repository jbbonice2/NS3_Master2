import 'package:flutter/material.dart';

class AppTheme {
  static const primaryColor = Color(0xFFbf9b7a);
  static const secondaryColor = Color(0xFFbf9b7a);
  static const backgroundColor = Color(0xFFf6f7fb);
  static const cardColor = Color(0xFFFFFFFF);
  static const unselectedColor = Color(0xFF675860);

  static ThemeData lightTheme = ThemeData(
    primaryColor: primaryColor,
    scaffoldBackgroundColor: backgroundColor,
    appBarTheme: const AppBarTheme(
      backgroundColor: primaryColor,
      foregroundColor: Colors.white,
    ),
    textTheme: const TextTheme(
      bodyMedium: TextStyle(color: Colors.black),
    ),
  );
}
