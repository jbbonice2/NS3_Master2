import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:mvp/helpers/widgets/custom_bottom_nav_bar.dart';

class GuideScreen extends StatefulWidget {
  const GuideScreen({super.key});

  @override
  _GuideScreenState createState() => _GuideScreenState();
}

class _GuideScreenState extends State<GuideScreen> {
  int _selectedIndex = 2;

  void _onItemTapped(int index) {
    setState(() {
      _selectedIndex = index;
    });

    switch (index) {
      case 1:
        Get.toNamed('/home');
        break;
      case 0:
        Get.toNamed('/today');
        break;
      case 2:
        Get.toNamed('/guide');
        break;
      case 3:
        Get.toNamed('/menu');
        break;
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF6F7FB),
      body: Center(
        child: Text(
          'Guide',
          style: Theme.of(context).textTheme.titleLarge,
        ),
      ),
      bottomNavigationBar: CustomBottomNavBar(
        selectedIndex: _selectedIndex,
        onItemSelected: _onItemTapped,
      ),
    );
  }
}
