# Tímový projekt

Tímový projekt – FIIT STU, 2021/2022.  
Téma: VANET siete  
Pedagogický vedúci: doc. Peter Trúchly  

Bezpečnosť v rámci automobilovej dopravy patrí medzi aktuálne témy,  ktorej sa venuje stále väčšia pozornosť. V rámci rôznych výskumov sa skúmajú možnosti komunikácie medzi vozidlami navzájom a medzi vozidlami a okolitými objektmi ako sú napríklad prijímacie veže, rôzne vysielače a iné zariadenia. Existujú rozličné nástroje, prostredníctvom ktorých je možné nasimulovať rôzne druhy komunikácií v rôznych dopravných situáciách. V rámci tejto práce sa zameriame na skúmanie možností týchto nástrojov.
Predovšetkým sa využíva nástroj NS-3 a jeho doplnky, nástroje SUMO, DOCKER kontajnerizácia a pod.

Repozitár obsahuje nami testované scenáre pre jednotlivé technológie používané vo VANET sieťach radené tematicky do priečinkov. Zaradené sú taktiež scenáre, ktoré nemohli byť vo finálnej verzií použité, z dôvodov opísaných v dokumentácií projektu.

## Obsah repozitára - vetva main
LTE_NS3 - scenáre s implementovanou LTE prenosovou technológiou<br>
P2 - finálny scenár s prenosovými technológiami Wi-Fi a 4G<br>
SDN - scenáre so sieťou SDN v rôznych topológiach<br>
Sumo_to_NS3_using_NS2 - zdrojové súbory pre konvertovanie dát z NS-2 do NS-3<br>
FunkcnaVerzia-ofswitch13-queue.cc.txt - súbor, ktorý podľa dokumentácie je potrebné zmeniť, aby SDN sieť bola funkčná na verzii 3.30


## Obsah repozitára - vetva backup
Vetva slúži pre zálohovanie scenárov, ktoré viedli k finálneho scenáru.<br>
Flow Monitor - vysvetlený postup pre impelemntáciu funkcionality FlowMonitor<br>
LTE_NS3 - scenáre s implementovanou LTE prenosovou technológiou<br>
Sumo_to_NS3_using_NS2 - zdrojové súbory pre konvertovanie dát z NS-2 do NS-3<br>
TraceMetrics - obsahuje PDF tutoriál pre TraceMetrics a ukážkové súbory prenosov<br>
lora_final.cc - obsahuje scenár s implementáciou lora prenosovej technológie<br>
lora_final_AsciiTraceHelper - lora scenar v ktorom je implementovaný štandart testovania<br>
wave-test.cc - scenár s wavenet prenosovou technológiou<br>
wifi-adhoc.cc - scenár s wifi ako adhoc siet<br>
wifi_standard_with_static_AP.cc - scenár s wifi komunikáciou pri statickej topológií s AP<br>
