# Pflanzenpflege

Eine Kachel für eine Pflanze. Sie zeigt, wann die Pflanze zuletzt gegossen und zuletzt gedüngt wurde, jeweils in einer
Zeile mit einem Punkt, der von grün über orange zu rot wechselt, wenn die Pflanze dran ist. Ein Tipp auf eine Zeile
auf dem Display trägt „heute“ ein.

## Einrichten

1. Füge das Plugin in Tessera zu einem Screen hinzu. Der Screen wird einmal gebaut.
2. Platziere im Layout die Kachel **Pflanze** aus der Gruppe Plugins der Bibliothek.
3. Gib im Inspector den Namen der Pflanze an und wie oft sie gegossen und gedüngt wird (in Tagen). Der Punkt ist grün,
   bis so viele Tage vergangen sind, danach etwa halb so lange orange, dann rot.

## Auf dem Display

- Tippe auf die **obere Zeile**, wenn du gegossen hast, auf die **untere Zeile**, wenn du gedüngt hast. Eingetragen
  wird der heutige Tag.
- Ein Tipp auf den Namen macht nichts, damit ein versehentliches Berühren kein Datum ändert.

## Gut zu wissen

- Die beiden Daten sind Entitäten des Screens in Home Assistant, **Plant last watered** und **Plant last fertilized**
  (Entitäten vom Typ Datum). Dort kannst du einen früheren Tag setzen, wenn du das Tippen vergessen hast. Sie bleiben
  auf dem Screen gespeichert und überstehen einen Neustart.
- Ein Screen hat eine Pflanze: Zwei Pflanzen-Kacheln auf demselben Screen zeigen dieselben zwei Daten.
- Die Kachel zählt nach der Uhr des Screens und springt um Mitternacht weiter. Solange Home Assistant die Uhr nicht
  gestellt hat, bleiben die Zeilen leer.
