# Simulation LoRaWAN avec sélecteur ToW MAB en environnement urbain

## Description

Cette simulation implémente un réseau LoRaWAN en **environnement urbain** utilisant l'algorithme Tug-of-War Multi-Armed Bandit (ToW MAB) pour la sélection adaptative du canal et du facteur d'étalement (SF). L'implémentation est basée sur l'article scientifique arXiv:2208.01824v1.

## Caractéristiques de l'environnement urbain

- **Portée limitée** : 2-4 km selon le dispositif
- **Nombreux obstacles** : Bâtiments, structures métalliques, véhicules
- **Interférences importantes** : Forte densité de dispositifs et d'autres technologies sans fil
- **Taux de succès variable** : Jusqu'à 85% pour les transmissions avec SF élevés
- **Interférences aléatoires** : Simulation d'interférences urbaines imprévisibles

## Paramètres de simulation

### Réseau
- **Nombre de dispositifs** : 10 (densité plus élevée typique d'un déploiement urbain)
- **Durée de simulation** : 100 secondes
- **Intervalle entre paquets** : 10 secondes

### Paramètres LoRaWAN
- **Bande de fréquence** : EU868 (868 MHz)
- **Canaux** : 8 canaux (868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9 MHz)
- **Facteurs d'étalement** : SF7 à SF12
- **Puissance d'émission** : 14-18 dBm
- **Bande passante** : 125 kHz
- **Coding Rate** : 4/5
- **Taille des paquets** : 20-47 octets

### Paramètres du sélecteur ToW MAB
- **Alpha (α)** : 0.9 (facteur de remise pour les Q-values)
- **Beta (β)** : 0.99 (facteur d'oubli pour N et R)
- **Amplitude d'oscillation** : 0.1
- **Nombre maximum de retransmissions** : 3
- **Intervalle de transmission** : 10.0 secondes

## Modèle de propagation

Le modèle de propagation est adapté pour représenter un environnement urbain :
- Probabilité de succès de base : 60%
- Bonus de probabilité pour les SF élevés : +5% par niveau de SF
- Probabilité maximale : 85%
- Facteur d'interférence aléatoire : 0-15% (simule les interférences urbaines)

## Exécution de la simulation

Pour exécuter la simulation, utilisez la commande suivante :

```bash
./ns3 run scratch/lorawan-tow-mab-urban
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

En environnement urbain, on s'attend à observer :
1. Une utilisation plus fréquente des facteurs d'étalement élevés (SF10-SF12)
2. Un taux d'échec plus important qu'en milieu rural
3. Une adaptation plus dynamique aux interférences
4. Une exploration plus intensive des différents canaux
5. Un taux de succès global entre 65% et 75%

## Défis spécifiques à l'environnement urbain

- **Effet canyon urbain** : Réflexions multiples entre les bâtiments
- **Interférences multisources** : WiFi, Bluetooth, GSM, autres réseaux LoRaWAN
- **Obstacles dynamiques** : Véhicules, foules
- **Variations temporelles** : Différences jour/nuit dans les niveaux d'interférence

## Extensions possibles

- Ajout d'un modèle de propagation urbain (OkumuraHataPropagationLossModel)
- Intégration d'un modèle de bâtiments (BuildingsPropagationLossModel)
- Simulation de la densité variable du trafic selon l'heure
- Modélisation de la consommation énergétique
- Comparaison avec d'autres algorithmes de sélection (UCB1, ε-greedy)
- Test avec différentes densités de dispositifs

## Références

- Article original sur ToW MAB : arXiv:2208.01824v1
- Spécification LoRaWAN : LoRa Alliance TS001-1.0.4
- Documentation NS-3 : https://www.nsnam.org/docs/
- Études sur la propagation LoRa en milieu urbain : Bor et al. "Do LoRa Low-Power Wide-Area Networks Scale?"
