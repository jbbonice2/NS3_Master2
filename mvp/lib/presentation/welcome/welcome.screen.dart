import 'package:flutter/material.dart';
import 'package:get/get.dart';

class WelcomeScreen extends StatelessWidget {
  const WelcomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.blue.shade900, // Fond bleu
      body: Column(
        mainAxisAlignment: MainAxisAlignment.spaceBetween, // Espacement entre les blocs
        children: [
          const SizedBox(height: 40), // Espace pour éviter le chevauchement avec la barre de statut
          Expanded(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center, // Centre le contenu au milieu
              children: [
                const AnimatedText(), // Message animé
                const SizedBox(height: 50), // Espacement
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    LanguageButton(
                      text: 'Français',
                      flag: 'assets/images/france_flag.png',
                      onTap: () => Get.toNamed('/home'), // Redirige vers Home
                    ),
                    LanguageButton(
                      text: 'English',
                      flag: 'assets/images/united_kingdom_flag.png',
                      onTap: () => Get.toNamed('/home'), // Redirige vers Home
                    ),
                  ],
                ),
              ],
            ),
          ),
          const Padding(
            padding: EdgeInsets.only(bottom: 20), // Ajout d'un espace en bas
            child: Text(
              'Knowledge',
              style: TextStyle(
                fontFamily: 'Ubuntu',
                fontSize: 24,
                fontWeight: FontWeight.bold,
                color: Colors.white, // Texte blanc
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class AnimatedText extends StatefulWidget {
  const AnimatedText({super.key});

  @override
  _AnimatedTextState createState() => _AnimatedTextState();
}

class _AnimatedTextState extends State<AnimatedText> {
  String text = '';
  final String fullText = 'Knowledge, Parce que tout commence par la connaissance !';
  int index = 0;

  @override
  void initState() {
    super.initState();
    _animateText();
  }

  void _animateText() {
    Future.delayed(const Duration(milliseconds: 100), () {
      if (index < fullText.length) {
        setState(() {
          text += fullText[index];
          index++;
        });
        _animateText();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Text(
      text,
      textAlign: TextAlign.center,
      style: const TextStyle(
        fontSize: 18,
        color: Colors.white, // Texte blanc
      ),
    );
  }
}

class LanguageButton extends StatelessWidget {
  final String text;
  final String flag;
  final VoidCallback onTap;

  const LanguageButton({
    super.key,
    required this.text,
    required this.flag,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return ElevatedButton(
      onPressed: onTap,
      style: ElevatedButton.styleFrom(
        padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12),
        backgroundColor: Colors.white,
        foregroundColor: Colors.blue.shade900, // Texte et icône bleu
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
        ),
        elevation: 5,
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Image.asset(flag, width: 30, height: 30),
          const SizedBox(width: 10),
          Text(
            text,
            style: const TextStyle(fontSize: 16),
          ),
        ],
      ),
    );
  }
}
