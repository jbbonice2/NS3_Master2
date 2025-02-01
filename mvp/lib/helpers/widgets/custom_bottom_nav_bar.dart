import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:bottom_navy_bar/bottom_navy_bar.dart';

class CustomBottomNavBar extends StatelessWidget {
  final int selectedIndex;
  final Function(int) onItemSelected;

  const CustomBottomNavBar({
    super.key,
    required this.selectedIndex,
    required this.onItemSelected,
  });

  @override
  Widget build(BuildContext context) {
    return BottomNavyBar(
      selectedIndex: selectedIndex,
      showElevation: true,
      onItemSelected: onItemSelected,
      items: [
        BottomNavyBarItem(
          icon: const Icon(Icons.home),
          title: const Text('Accueil'),
          activeColor: const Color(0xFFbf9b7a),
          inactiveColor: const Color(0xFF675860),
        ),
        BottomNavyBarItem(
          icon: const Icon(Icons.calendar_today),
          title: const Text('Aujourd\'hui'),
          activeColor: const Color(0xFFbf9b7a),
          inactiveColor: const Color(0xFF675860),
        ),
        BottomNavyBarItem(
          icon: const Icon(Icons.book),
          title: const Text('Guide'),
          activeColor: const Color(0xFFbf9b7a),
          inactiveColor: const Color(0xFF675860),
        ),
        BottomNavyBarItem(
          icon: const Icon(Icons.menu),
          title: const Text('Menu'),
          activeColor: const Color(0xFFbf9b7a),
          inactiveColor: const Color(0xFF675860),
        ),
      ],
    );
  }
}
