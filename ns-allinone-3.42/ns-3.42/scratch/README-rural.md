# Simulation LoRaWAN avec sélecteur ToW MAB en environnement rural

## Description

Cette simulation implémente un réseau LoRaWAN en **environnement rural** utilisant l'algorithme Tug-of-War Multi-Armed Bandit (ToW MAB) pour la sélection adaptative du canal et du facteur d'étalement (SF). L'implémentation est basée sur l'article scientifique arXiv:2208.01824v1.

## Caractéristiques de l'environnement rural

- **Portée étendue** : 10-14 km selon le dispositif
- **Moins d'obstacles** : Meilleure propagation du signal
- **Moins d'interférences** : Densité plus faible de dispositifs et d'autres technologies sans fil
- **Taux de succès plus élevé** : Jusqu'à 95% pour les transmissions avec SF élevés

## Paramètres de simulation

### Réseau
- **Nombre de dispositifs** : 5 (densité typique d'un déploiement rural)
- **Durée de simulation** : 100 secondes
- **Intervalle entre paquets** : 10 secondes

### Paramètres LoRaWAN
- **Bande de fréquence** : EU868 (868 MHz)
- **Canaux** : 8 canaux (868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9 MHz)
- **Facteurs d'étalement** : SF7 à SF12
- **Puissance d'émission** : 14-18 dBm
- **Bande passante** : 125 kHz
- **Coding Rate** : 4/5
- **Taille des paquets** : 20-40 octets

### Paramètres du sélecteur ToW MAB
- **Alpha (α)** : 0.9 (facteur de remise pour les Q-values)
- **Beta (β)** : 0.99 (facteur d'oubli pour N et R)
- **Amplitude d'oscillation** : 0.1
- **Nombre maximum de retransmissions** : 3
- **Intervalle de transmission** : 10.0 secondes

## Modèle de propagation

Le modèle de propagation est simplifié pour représenter un environnement rural :
- Probabilité de succès de base : 80%
- Bonus de probabilité pour les SF élevés : +3% par niveau de SF
- Probabilité maximale : 95%

## Exécution de la simulation

Pour exécuter la simulation, utilisez la commande suivante :

```bash
./ns3 run scratch/lorawan-tow-mab-rural
```

## Résultats attendus

La simulation affiche pour chaque transmission :
- L'identifiant du dispositif
- Le canal et la fréquence utilisés
- Le facteur d'étalement
- La puissance d'émission
- La bande passante
- Le coding rate
- La taille du paquet
- La portée estimée
- Le résultat de la transmission (succès ou échec)

À la fin de la simulation, un résumé affiche :
- Le nombre total de tentatives de transmission
- Le nombre total de transmissions réussies
- Le taux global de succès (FSR - Frame Success Rate)

## Analyse des performances

En environnement rural, on s'attend à observer :
1. Une convergence vers des facteurs d'étalement plus bas (SF7-SF9) pour les dispositifs proches
2. L'utilisation de SF plus élevés (SF10-SF12) pour les dispositifs plus éloignés
3. Un taux de succès global supérieur à 80%
4. Une exploration efficace des différents canaux pour éviter les interférences

## Extensions possibles

- Ajout d'un modèle de propagation plus réaliste (LogDistancePropagationLossModel)
- Intégration de la mobilité des dispositifs
- Simulation de conditions météorologiques variables
- Modélisation de la consommation énergétique
- Comparaison avec d'autres algorithmes de sélection (UCB1, ε-greedy)

## Références

- Article original sur ToW MAB : arXiv:2208.01824v1
- Spécification LoRaWAN : LoRa Alliance TS001-1.0.4
- Documentation NS-3 : https://www.nsnam.org/docs/
