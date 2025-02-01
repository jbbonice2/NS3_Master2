import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:mvp/helpers/widgets/custom_bottom_nav_bar.dart';


import 'package:flutter/material.dart';

import 'package:flutter/material.dart';

class TextPage extends StatefulWidget {
  final Map<String, dynamic> law;

  const TextPage({super.key, required this.law});

  @override
  _TextPageState createState() => _TextPageState();
}

class _TextPageState extends State<TextPage> with SingleTickerProviderStateMixin {
  List<dynamic> currentList = [];
  String? currentContent;
  late TabController _tabController;

  @override
  void initState() {
    super.initState();
    currentList = widget.law['chapters'];
    _tabController = TabController(length: 2, vsync: this);
  }

  void onChapterSelected(Map<dynamic, dynamic> chapter) {
    setState(() {
      currentList = chapter['articles'];
      currentContent = null;
    });
  }

  void onArticleSelected(Map<dynamic, dynamic> article) {
    setState(() {
      currentList = article['paragraphs'];
      currentContent = article['content'];
    });
  }

  @override
  void dispose() {
    _tabController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF6F7FB),
      appBar: AppBar(
        title: Text(widget.law['title']),
        backgroundColor: const Color(0xFFbf9b7a),
        bottom: TabBar(
          controller: _tabController,
          tabs: const [
            Tab(icon: Icon(Icons.text_snippet), text: "Texte Complet"),
            Tab(icon: Icon(Icons.grid_view), text: "Navigation"),
          ],
        ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: [
          // Première colonne : Texte complet
          SingleChildScrollView(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: widget.law['chapters'].map<Widget>((chapter) {
                return Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      chapter['title'],
                      style: const TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                        color: Color(0xFF675860),
                      ),
                    ),
                    ...chapter['articles'].map<Widget>((article) {
                      return Padding(
                        padding: const EdgeInsets.symmetric(vertical: 8.0),
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Text(
                              article['title'],
                              style: const TextStyle(
                                fontSize: 16,
                                fontWeight: FontWeight.w600,
                                color: Color(0xFFbf9b7a),
                              ),
                            ),
                            Text(
                              article['content'],
                              style: const TextStyle(fontSize: 14),
                            ),
                            ...article['paragraphs'].map<Widget>((paragraph) {
                              return Padding(
                                padding: const EdgeInsets.only(left: 16.0),
                                child: Text(
                                  "- $paragraph",
                                  style: const TextStyle(fontSize: 14),
                                ),
                              );
                            }).toList(),
                          ],
                        ),
                      );
                    }).toList(),
                  ],
                );
              }).toList(),
            ),
          ),

          // Deuxième colonne : Navigation en grille
          GridView.builder(
            padding: const EdgeInsets.all(16.0),
            gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
              crossAxisCount: 2,
              mainAxisSpacing: 16.0,
              crossAxisSpacing: 16.0,
            ),
            itemCount: currentList.length,
            itemBuilder: (context, index) {
              final item = currentList[index];
              return GestureDetector(
                onTap: () {
                  if (item is Map) {
                    if (item.containsKey('articles')) {
                      onChapterSelected(item);
                    } else if (item.containsKey('paragraphs')) {
                      onArticleSelected(item);
                    }
                  }
                },
                child: Card(
                  color: Colors.white,
                  elevation: 4,
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: Center(
                    child: Text(
                      item is Map ? item['title'] : item,
                      textAlign: TextAlign.center,
                      style: const TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 14,
                        color: Color(0xFF675860),
                      ),
                    ),
                  ),
                ),
              );
            },
          ),
        ],
      ),
    );
  }
}



class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  _HomeScreenState createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  int _selectedIndex = 1;
  static const List<Map<String, dynamic>> lawTexts = [
  {
    "title": "Constitution",
    "chapters": [
      {
        "title": "Chapitre 1",
        "articles": [
          {
            "title": "Article 1",
            "content": "Texte de l'article 1",
            "paragraphs": ["Alinéa 1", "Alinéa 2"]
          },
          {
            "title": "Article 2",
            "content": "Texte de l'article 2",
            "paragraphs": ["Alinéa 1", "Alinéa 2", "Alinéa 3"]
          }
        ]
      },
      {
        "title": "Chapitre 2",
        "articles": [
          {
            "title": "Article 3",
            "content": "Texte de l'article 3",
            "paragraphs": ["Alinéa 1"]
          }
        ]
      }
    ]
  },
  {
    "title": "Code Pénal",
    "chapters": [
      {
        "title": "Chapitre 1",
        "articles": [
          {
            "title": "Article 1",
            "content": "Texte de l'article 1",
            "paragraphs": ["Alinéa 1", "Alinéa 2"]
          }
        ]
      }
    ]
  }
];


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
      appBar: AppBar(
        title: const Text("Textes Disponibles"),
        backgroundColor: const Color(0xFFbf9b7a),
      ),
      body: ListView.builder(
        itemCount: lawTexts.length,
        itemBuilder: (context, index) {
          final law = lawTexts[index];
          return GestureDetector(
            onTap: () {
              Get.to(() => TextPage(law: law));
            },
            child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,

                children: [Card(
                  margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                  elevation: 4,
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(12),
                  ),
                  child: ListTile(
                    title: Text(
                      law['title'],
                      textAlign: TextAlign.center,
                      style: const TextStyle(fontWeight: FontWeight.bold),
                    ),
                    // trailing: const Icon(Icons.arrow_forward_ios),
                  ),
                ),
                ]
              )
          );
        },
      ),
      bottomNavigationBar: CustomBottomNavBar(
        selectedIndex: _selectedIndex,
        onItemSelected: _onItemTapped,
      ),
    );
  }
}
