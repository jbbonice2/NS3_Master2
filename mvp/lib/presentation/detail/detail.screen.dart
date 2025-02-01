import 'package:flutter/material.dart';
import 'package:get/get.dart';

class DetailScreen extends StatefulWidget {
  const DetailScreen({super.key});

  @override
  _DetailScreenState createState() => _DetailScreenState();
}

class _DetailScreenState extends State<DetailScreen> with TickerProviderStateMixin {
  late TabController _tabController;
  final List<String> chapters = ['Chapitre 1', 'Chapitre 2', 'Chapitre 3'];
  final Map<String, List<String>> articles = {
    'Chapitre 1': ['Alinéa 1', 'Alinéa 2', 'Alinéa 3'],
    'Chapitre 2': ['Alinéa 1', 'Alinéa 2'],
    'Chapitre 3': ['Alinéa 1'],
  };

  String selectedChapter = 'Chapitre 1';

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: 2, vsync: this);
  }

  @override
  Widget build(BuildContext context) {
    final String title = Get.arguments['title'] ?? 'Détail';

    return Scaffold(
      appBar: AppBar(
        title: Text(title),
        backgroundColor: const Color(0xFFbf9b7a),
        bottom: TabBar(
          controller: _tabController,
          tabs: const [
            Tab(text: 'Texte'),
            Tab(text: 'Navigation'),
          ],
        ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: [
          // Texte avec contenu complet
          SingleChildScrollView(
            padding: const EdgeInsets.all(16.0),
            child: Text(
              'Contenu complet de $selectedChapter',
              style: const TextStyle(fontSize: 16),
            ),
          ),

          // Navigation entre chapitres et alinéas
          Row(
            children: [
              // Liste des chapitres
              Expanded(
                flex: 2,
                child: ListView.builder(
                  itemCount: chapters.length,
                  itemBuilder: (context, index) {
                    return ListTile(
                      title: Text(chapters[index]),
                      onTap: () {
                        setState(() {
                          selectedChapter = chapters[index];
                        });
                      },
                      selected: chapters[index] == selectedChapter,
                    );
                  },
                ),
              ),

              // Liste des alinéas du chapitre sélectionné
              Expanded(
                flex: 3,
                child: ListView.builder(
                  itemCount: articles[selectedChapter]?.length ?? 0,
                  itemBuilder: (context, index) {
                    final String article = articles[selectedChapter]![index];
                    return ListTile(
                      title: Text(article),
                      onTap: () {
                        Get.snackbar(
                          'Alinéa sélectionné',
                          '$article dans $selectedChapter',
                          snackPosition: SnackPosition.BOTTOM,
                        );
                      },
                    );
                  },
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
