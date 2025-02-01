import 'package:flutter/material.dart';
import 'package:get/get.dart';

class SplashScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.blue.shade900, // Fond bleu épais
      body: Center(
        child: ZoomAnimation(),
      ),
    );
  }
}

class ZoomAnimation extends StatefulWidget {
  @override
  _ZoomAnimationState createState() => _ZoomAnimationState();
}

class _ZoomAnimationState extends State<ZoomAnimation>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _scaleAnimation;

  @override
  void initState() {
    super.initState();

    _controller = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 3), // Durée de l'animation
    );

    _scaleAnimation = Tween<double>(begin: 0.2, end: 1.0).animate(
      CurvedAnimation(parent: _controller, curve: Curves.easeInOut),
    );

    _controller.forward();

    // Naviguer vers l'écran suivant après la fin de l'animation
    Future.delayed(const Duration(seconds: 4), () {
      Get.offNamed('/welcome');
    });
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (context, child) {
        return Transform.scale(
          scale: _scaleAnimation.value,
          child: Opacity(
            opacity: _controller.value,
            child: Image.asset(
              'assets/images/cameroon_flag.png',
              width: 200,
              height: 200,
            ),
          ),
        );
      },
    );
  }
}
