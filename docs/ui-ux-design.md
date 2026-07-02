# GrooveS3 — Design UI/UX Completo

Sessione di design: 2026-06-30, aggiornato 2026-07-02. Nessun codice scritto — design puro, da implementare in Phase 4.

Questo documento è la spec di riferimento per l'implementazione delle schermate, della navigazione e dei controlli. Non aggiungere funzionalità non previste qui senza prima aggiornare questo documento.

---

## Hardware UI (confermato per Phase 4)

| Componente | Quantità | Funzione |
|---|---|---|
| HC595 | ×2 in cascata | 16 step LED |
| HC165 | ×2 | 16 step buttons |
| HC165 | ×3 | 8 function buttons |
| Potenziometri | ×4 (GPIO 1–4) | POT1–POT4, contestuali per schermata |
| ENC1 | ×1 con click | Navigazione principale + azione primaria |
| ENC2 | ×1 con click | Parametro corrente — click/shift+click sempre contestuali alla schermata |
| SHIFT | ×1 button | Held modifier — MAI toggle |

### 8 Function Buttons (HC165 ×3)
- **FB1** — PLAY/STOP (SHIFT+FB1 = OVERVIEW da qualsiasi modalità)
- **FB2** — REC, sempre e ovunque (SHIFT+FB2 = toggle `[SEQ]` ↔ `[KEYS]`)
- **FB3** — MODE: OVERVIEW/PLAY
- **FB4** — MODE: PATTERN
- **FB5** — MODE: SOUND
- **FB6** — MODE: NOTE
- **FB7** — MODE: FX
- **FB8** — MODE: MIXER

---

## Sistema di Navigazione — 7 Modalità

```
OVERVIEW/PLAY  ←→  PATTERN  ←→  SOUND  ←→  NOTE  ←→  FX  ←→  MIXER
```

Ogni modalità ha una function button dedicata — navigazione diretta, non ciclica.

**Regole globali di navigazione:**
- `SHIFT + [PLAY/STOP]` (FB1) = OVERVIEW da qualsiasi modalità (priorità assoluta, sovrascrive tutto)
- `FB2` = REC sempre, qualsiasi modalità, nessuna eccezione (bottone fisico dedicato)
- `SHIFT + FB2` = toggle `[SEQ]` ↔ `[KEYS]`; default al boot: `[SEQ]` attivo; `[KEYS]` valido **solo nella schermata OVERVIEW/PLAY** (SHIFT+FB2 non ha effetto in nessun'altra schermata, resta forzato su `[SEQ]`)
- `ENC1/ENC2 click` e `SHIFT + ENC1/ENC2 click` = **sempre azioni contestuali alla schermata corrente** (mai un significato globale — vedi tabelle per modalità)
- `ENC1 click` fuori da PLAY = reset del parametro evidenziato al default, dove non esplicitamente ridefinito per schermata

---

## Schermata 1 — OVERVIEW / PLAY

Display: griglia 6×16, BPM, track attivo, indicatore REC. Tracce con lunghezze diverse: 16 colonne fisse, step oltre la lunghezza della traccia mostrati come trattini.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona traccia attiva (1–6) |
| ENC1 click | PLAY / STOP |
| ENC2 rotate | Naviga cursore statico sugli step (indipendente dal playhead) |
| ENC2 click | Toggle lo step sotto il cursore statico |
| SHIFT held | Overlay velocity lane sui 16 LED |
| SHIFT + PLAY/STOP | OVERVIEW (priorità assoluta, anche durante velocity lane) |
| POT1 | BPM (range 40–200) |
| POT2–4 | Disabilitati in OVERVIEW (read-only) |
| Step buttons ([SEQ]) | Attiva/disattiva step sulla traccia attiva |
| Step buttons ([KEYS]) | Tastiera cromatica (C→D# su 16 semitoni, ottava via ENC1) |
| SHIFT + step | Setta lunghezza loop sulla traccia attiva |

**Tap tempo:** solo in PLAY mode.

**Due cursori distinti:**
- LED → playhead (posizione riproduzione corrente)
- OLED → cursore statico (freccia/box, navigabile via ENC2 rotate)

### Velocity Lane (SHIFT held)
- SHIFT held → 16 LED = barre di velocity visive
- Step buttons → imposta velocity per quello step
- POT1–POT2–POT4 disabilitati; solo POT3 = velocity value del cursore
- `SHIFT + PLAY/STOP` → esce e va in OVERVIEW (priorità assoluta)

### TR-808 Live Recording
- REC attivo + PLAY + `[KEYS]`: nota registrata sullo step corrente del playhead (stile TR-808)
- Overdub: nuovi step si aggiungono, vecchi rimangono
- Premi step già acceso durante REC = cancella lo step

---

## Schermata 2 — PATTERN

Display: pattern corrente, lunghezza, swing, probability.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona pattern (1–16) |
| ENC1 click | Copia pattern corrente |
| ENC2 rotate | Cambia lunghezza pattern (1–16 step) |
| ENC2 click | Aggiungi/rimuovi pattern corrente dalla chain (toggle) |
| SHIFT + ENC1 rotate | Sposta pattern nella chain |
| SHIFT + ENC1 click | Cancella pattern |
| Step buttons | Jump diretto al pattern N |
| POT1 | BPM |
| POT2 | Swing amount |
| POT3 | PROBABILITY (0–100%) |
| POT4 | GATE LENGTH (fallback per tracce senza ARP attivo) |

**Nota GATE LENGTH:** quando ARP è attivo su una traccia, NOTE/ARP GATE ha priorità su PATTERN GATE LENGTH. OLED mostra `[ARP]` accanto a GATE LENGTH quando ARP è attivo.

### LED in PATTERN
- LED acceso = pattern ha note
- LED lampeggiante = pattern in riproduzione
- 16 LED mostrano tutti i 16 pattern con marker per quelli in chain

---

## Schermata 3 — SOUND

3 sub-schermate selezionate da ENC1 rotate: **DRUM** | **MELODIC** | **SAMPLE**

**ENC2 click in SOUND** = toggle tipo traccia (DRUM ↔ MELODIC) — cambia il tipo e fa redirect automatico con hint `→ SOUND` sull'OLED.

**SHIFT + ENC2 click in SOUND MELODIC** = cicla loop mode (OFF → LOOP → PING-PONG).

**SHIFT held in SOUND** = Envelope sub-view (ADSR grafico animato) per tutti i tipi di traccia.

### SOUND / DRUM

Display: nome voce PCM, preset selezionato, velocity, pitch offset.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona voce drum (kick/snare/hat/clap/tom/clave) |
| ENC1 click | Preview della voce |
| ENC2 rotate | Cicla preset PCM |
| ENC2 click | Toggle tipo traccia (DRUM→MELODIC) |
| SHIFT held | Envelope sub-view (ADSR) |
| SHIFT + ENC2 click | — (nessuna azione su DRUM) |
| POT1 | Pitch offset |
| POT2 | Decay |
| POT3 | Punch (attack transient) |
| POT4 | VOLUME (ampiezza oscillatore) |

### SOUND / MELODIC

Display: forma d'onda animata (modificata in real-time dai parametri), waveform type, envelope ADSR.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona parametro da modificare |
| ENC1 click | Reset parametro selezionato al default |
| ENC2 rotate | Modifica valore parametro selezionato |
| ENC2 click | Toggle tipo traccia (MELODIC→DRUM) |
| SHIFT held | Envelope sub-view (ADSR grafico) |
| SHIFT + ENC2 click | Cicla loop mode (OFF→LOOP→PING-PONG) |
| POT1 | Cutoff filter |
| POT2 | Resonance |
| POT3 | Detune / FM ratio |
| POT4 | VOLUME (ampiezza oscillatore) |

**Auto p-lock in Envelope sub-view:** REC attivo + SHIFT held → la registrazione p-lock è in pausa. OLED mostra `[REC PAUSED]`. Riprende al rilascio di SHIFT.

**Nessun preview sample da SOUND MELODIC** — ENC2 click è occupato dal toggle tipo. Preview disponibile solo da SAMPLE editor.

### SOUND / SAMPLE

Display: forma d'onda campione, marker cut-in/cut-out, nome file. Hint OLED contestuale: `SHIFT: ENV` / `SHIFT: EDIT`.

**ENC2 click in SOUND SAMPLE** — context-sensitive:
- Traccia DRUM: solo pagina 1 (CUT-IN/CUT-OUT), nessun ciclo
- Traccia MELODIC: cicla tra pagina 1 (CUT-IN/CUT-OUT) e pagina 2 (LOOP START/LOOP END)

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Sfoglia campioni su SD |
| ENC1 click | Carica campione selezionato |
| ENC2 rotate | Zoom waveform |
| ENC2 click | Cicla pagine (MELODIC) / nessun ciclo (DRUM) |
| SHIFT held | Envelope sub-view |
| SHIFT + ENC1 rotate | Sposta cut-in marker |
| SHIFT + ENC2 rotate | Sposta cut-out marker |
| POT1 | Cut-in fine-tune |
| POT2 | Cut-out fine-tune |
| POT3 | Pitch shift |
| POT4 | Volume campione |

**Cambio tipo traccia:** se l'utente è in NOTE o SAMPLE mode e cambia tipo traccia, redirect automatico a SOUND con hint `→ SOUND` sull'OLED.

**Limite accettato:** il toggle DRUM↔MELODIC (ENC2 click) non è raggiungibile dalla sotto-schermata SAMPLE, dove ENC2 click è occupato da "cicla pagine". Per cambiare tipo traccia da SAMPLE, l'utente deve prima tornare a DRUM o MELODIC (ENC1 rotate).

---

## Schermata 4 — NOTE

3 sub-schermate: **MONO** | **CHORD** | **ARP**

ENC1 rotate naviga tra le sub-schermate.
NOTE mode disabilitato per tracce DRUM.

### NOTE / MONO

Display: piano roll lineare 16 step, nota corrente, ottava, scala attiva.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Naviga ottava su/giù |
| ENC1 click | Reset ottava al default |
| ENC2 rotate | Cicla scala (scale root + mode) |
| ENC2 click | Preview: suona la nota corrente |
| SHIFT + ENC2 click | QUANTIZE on/off (snap note alla scala corrente) |
| POT1 | VELOCITY DEFAULT (per tutti i nuovi step) |
| POT2 | GATE LENGTH |
| POT3 | HUMANIZE (variazione casuale velocity, 0 = nessuna) |
| POT4 | TRANSPOSE per traccia (±12 semitoni) |
| Step buttons | Assegna nota corrente allo step / toggle step |

**`[KEYS]` non disponibile in NOTE/MONO** — `[KEYS]` è valido solo nella schermata OVERVIEW/PLAY (vedi regola globale). Qui gli step buttons lavorano sempre in modalità `[SEQ]`.

### NOTE / CHORD

Display: voicing accordo, tipo accordo, inversione.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona inversione voicing (root/1st/2nd) |
| ENC1 click | Reset al default |
| ENC2 rotate | Cicla tipo accordo (maj/min/7/maj7/sus2/sus4…) |
| ENC2 click | Preview: suona l'accordo corrente |
| SHIFT + ENC2 click | QUANTIZE on/off |
| POT1 | VELOCITY DEFAULT |
| POT2 | GATE LENGTH |
| POT3 | Root note accordo |
| POT4 | Spread (apertura voicing) |
| Step buttons | Assegna accordo corrente allo step |

### NOTE / ARP

Display: pattern ARP, rate, ottave, direzione, step attivi.

Step attivi nel sequencer = quando l'arpeggiatore viene triggerato. Step ON = arp parte su quel beat.

| Controllo | Funzione |
|---|---|
| ENC1 rotate | ARP rate (1/4, 1/8, 1/16, 1/32) |
| ENC1 click | Reset al default |
| ENC2 rotate | Cicla pattern ARP: UP / DOWN / UP-DOWN / RANDOM / CHORD |
| ENC2 click | Preview: suona il pattern arpeggio corrente |
| SHIFT + ENC2 click | QUANTIZE on/off |
| POT1 | VELOCITY DEFAULT |
| POT2 | GATE LENGTH ARP |
| POT3 | Ottave (1–4) |
| POT4 | — |

**ARP nota sorgente:** le note della scala impostata in NOTE/MONO (root + scala + ottave da NOTE/ARP).

**ARP vs GATE LENGTH:** NOTE/ARP GATE ha priorità su PATTERN GATE LENGTH quando ARP è attivo. OLED mostra `[ARP]` accanto a GATE LENGTH.

**Scala di default:** C Major.

> **Nota naming:** il pattern ARP chiamato "CHORD" (tutte le note insieme) è distinto dalla sub-schermata NOTE/CHORD. Il contesto (si è in ARP) disambigua sempre.

---

## Schermata 5 — FX (8 effetti)

ENC1 rotate naviga tra gli 8 effetti. Ogni effetto ha una schermata dedicata con **estetica arcade anni 80**.

| Slot | Effetto | Visual arcade |
|---|---|---|
| FX1 | Reverb | Caverna con asteroidi rimbalzanti |
| FX2 | Delay | Tunnel spaziale con echo visivo |
| FX3 | Chorus | Forme sinusoidali che si sdoppiano |
| FX4 | Distortion | Glitch pixelato stile Pac-Man |
| FX5 | Filter (LP/HP/BP) | Radar che scansiona frequenze |
| FX6 | Bitcrusher | Pixel che si distruggono |
| FX7 | Compressor | Barre che si schiacciano come Breakout |
| FX8 | EQ (3 bande) | Equalizzatore stile Space Invaders |

**Sistema misto:** send amount per traccia (individuale); parametri effetto globali (condivisi).

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona effetto (1–8) |
| ENC1 click | Assegna effetto alla traccia corrente |
| ENC2 rotate | Modifica parametro principale dell'effetto |
| ENC2 click | Randomize parametri effetto corrente (POT2/3/4) |
| SHIFT held | Mostra secondo layer parametri FX (room size, feedback, ecc.) |
| SHIFT + ENC2 click | Bypass on/off dell'effetto corrente |
| POT1 | Wet/Dry mix |
| POT2 | Parametro 2 (es. Reverb: decay; Delay: feedback) |
| POT3 | Parametro 3 (es. Reverb: pre-delay; Delay: time) |
| POT4 | FX Send amount |

**LED fuori PLAY:** sempre mostrano il pattern della traccia corrente.

---

## Schermata 6 — MIXER

Display: 6 canali, fader, pan, EQ visivo per canale selezionato.

**MIXER non ha sends** — sends appartengono alla schermata FX.
**MIXER ha EQ per canale** (HIGH/LOW, non sends).

| Controllo | Funzione |
|---|---|
| ENC1 rotate | Seleziona canale (traccia 1–6) |
| ENC1 click | Mute canale selezionato (immediato, no hold) |
| ENC2 rotate | Modifica il parametro selezionato |
| ENC2 click | Cicla il parametro bersaglio di ENC2 rotate (Volume→Pan→High EQ→Low EQ) |
| SHIFT + ENC1 click | Solo canale selezionato |
| POT1 | Volume canale |
| POT2 | Pan |
| POT3 | HIGH EQ |
| POT4 | LOW EQ (Master volume sul POT4 globale — da definire in impl.) |

**Mute/Solo:**
- Mute/Solo immediati sulla traccia corrente (nessun hold)
- Solo = toggle (ripremi su traccia in solo = disattiva)
- Multi-solo possibile (più tracce in solo insieme)
- SOLO ha priorità su MUTE; premere SOLO su traccia mutata rimuove il mute
- SOLO + MUTE sulla stessa traccia → SOLO vince

---

## Parameter Locks (P-Locks)

P-lock registra per ogni step valori diversi da quelli base della traccia.

| Parametro | Valore |
|---|---|
| PITCH | Nota/pitch per quello step |
| CUTOFF | Cutoff filter per quello step |
| GATE | Durata nota per quello step |
| FX SEND | Send amount verso l'effetto assegnato alla traccia |

**FX SEND p-lock:** si riferisce sempre all'effetto attualmente assegnato alla traccia. OLED mostra `FX:[nome effetto]` nel tooltip.

**Feedback visivo p-lock:** step con lock mostrati con simbolo `▒` sull'OLED (invece di rettangolo pieno).

**Come registrare p-lock:**
1. Manuale: `hold [REC]` (= tieni premuto **FB2**) `+ step button` → 4 pot temporaneamente = PITCH/CUTOFF/GATE/FX SEND per quello step
2. Auto: REC attivo + PLAY → ogni step che il playhead attraversa riceve i valori pot correnti come lock
3. Auto pausa: in SOUND + SHIFT held (Envelope sub-view) → OLED mostra `[REC PAUSED]`

**P-lock valido solo in [SEQ] mode** (mutualmente esclusivo con [KEYS] mode).

---

## LED Behavior

| Stato | LED |
|---|---|
| Step vuoto | Spento |
| Step con nota | Acceso |
| Step attivo in riproduzione (su step vuoto) | Flash ON — blip digitale |
| Step attivo in riproduzione (su step con nota) | Flash OFF — blip digitale |
| Loop marker | LED lampeggiante lento sul bordo del loop |
| Velocity lane (SHIFT held) | 16 LED = barre di velocity proporzionali |
| Modalità PATTERN | LED mostrano track length; LED oltre lunghezza = spenti |
| LED fuori PLAY mode | Sempre mostrano pattern della traccia corrente |

**Blip cursor:** durata max(30% del passo, 40ms). Zero flicker a tutti i BPM.

**OVERVIEW con tracce a lunghezze diverse:** 16 colonne fisse. Step oltre la lunghezza della traccia = trattini.

---

## Track Types

| Tipo | Voci | NOTE_OFF | Note |
|---|---|---|---|
| DRUM | PCM (preset 1–6) | MAI (taglia sample) | `is_pcm = true` nella struct Track |
| MELODIC | AMY oscillatori | Sì | FM, sine, saw, Karplus-Strong |
| SAMPLE | SD card | Sì (con fade) | Cut-in/cut-out configurabili |

NOTE mode disabilitato per tracce DRUM. SAMPLE editor pagina 2 (LOOP) disponibile solo per MELODIC.

---

## Boot State

- Traccia 1 selezionata
- BPM 120
- Schermata OVERVIEW/PLAY
- Nessun pattern caricato (grid vuota)
- Volume master 0.75
- `[SEQ]` attivo di default
- Boot animation triggerata da Core 0 come ultimo step init, dopo flag da Core 1 che AMY è pronto

---

## Problemi Risolti — Log completo (80 problemi)

### Batch 1 — Prima ricognizione
1. ENC2 click conflict (REC vs bypass) → ENC2 click = REC sempre; SHIFT+ENC2 = bypass in FX
2. LED fuori PLAY mode → mostrano sempre il pattern della traccia corrente
3. Step buttons ambiguità → [SEQ] = sequencer; [KEYS] = tastiera cromatica
4. 7 modalità in ciclo → shortcut diretti con function buttons dedicati
5. SAMPLE mode su tracce melodiche → SAMPLE disponibile solo se traccia è tipo sample
6. NOTE mode su tracce DRUM → NOTE mode disabilitato per DRUM
7. Parameter locks non progettati → hold [REC] + step = 4 pot temporaneamente = PITCH/CUTOFF/GATE/FX SEND
8. REC in modalità non-PLAY → REC funziona solo in PLAY e SOUND mode
9. Vista multi-traccia assente → SHIFT+[PLAY/STOP] = OVERVIEW da qualsiasi modalità
10. Boot animation timing → triggerata da Core 0 dopo flag da Core 1

### Batch 2 — Seconda ricognizione
11. SHIFT in SOUND conflict (Envelope vs SAMPLE editor) → context-sensitive; hint OLED `SHIFT: ENV` / `SHIFT: EDIT`
12. P-lock + [KEYS] mode conflitto → mutualmente esclusivi; lock solo in [SEQ]
13. ENC2 click in PLAY/PATTERN/NOTE/FX non definito → PLAY=cursore/REC; PATTERN=REC; NOTE=REC; FX=REC
14. SHIFT+[PLAY/STOP] scope → accessibile da qualsiasi modalità
15. [MUTE]/[SOLO] meccanismo → immediato sulla traccia corrente, nessun hold (Opzione A)

### Batch 3 — Terza ricognizione
16. Mode switching automatico quando cambia tipo traccia → redirect automatico a SOUND con hint `→ SOUND`
17. ENC2 click PLAY "toggle step" problematico → ridefinito a REC
18. Velocity lane toggle vs held → held modifier (SHIFT held = velocity lane momentanea)
19. ENC2 rotate DRUM in SOUND → cicla preset PCM; MELODIC → cicla waveform
20. P-lock nessun feedback visivo → step con lock = `▒` sull'OLED
21. ARP + step sequencer interazione → step definiscono quando l'arp viene triggerato
22. ENC2 click in SOUND → ridefinito più volte (vedi batch 8 per versione finale)
23. POT durante OVERVIEW → pot disabilitati in OVERVIEW (read-only)

### Batch 4 — "Controlla bene tutto quanto"
24. ENC2 rotate in PLAY → naviga cursore statico sugli step
25. ENC2 rotate in NOTE sub-modes → modifica parametro primario discreto
26. ENC2 rotate in SAMPLE → movimento fine del marker attivo
27. ENC2 rotate in MIXER → modifica il parametro selezionato (pan/EQ)
28. Conflitto selettore DRUM/MELODIC vs ENC2 rotate → risolto in batch 8: ENC2 click = tipo toggle
29. POT1 = WAVE ridondante con ENC2 → POT1 in SOUND SYNTH = VOLUME (ampiezza)
30. ARP — quali note? → note della scala impostata in NOTE/MONO
31. [KEYS] + REC live recording → stile TR-808 su step del playhead corrente
32. Tap tempo → solo in PLAY mode
33. [SEQ] vs [KEYS] default → [SEQ] attivo di default al boot

### Batch 5 — "Fai di nuovo un check"
34. SHIFT toggle vs held → SHIFT = sempre held modifier, mai toggle
35. Due cursori (playhead + cursore statico) → LED = playhead; OLED = cursore statico
36. REC in SOUND → auto p-lock (step che il playhead attraversa riceve i valori pot correnti)
37. FX parametri globali o per traccia → misto: send per traccia; parametri effetto globali
38. PATTERN POT3/POT4 → POT3=PROBABILITY; POT4=GATE LENGTH
39. SOLO+MUTE stessa traccia → SOLO ha priorità; SOLO su traccia mutata rimuove il mute
40. OVERVIEW tracce lunghezze diverse → 16 colonne fisse; step oltre lunghezza = trattini

### Batch 6 — "Controlla ancora, rileva TUTTI i problemi"
41. Type selector DRUM/MELODIC accesso → ENC2 click in SOUND = toggle tipo
42. SAMPLE editor loop points pagina 2 → ENC2 click in SAMPLE cicla pagine (solo MELODIC)
43. [KEYS] ENC2 rotate conflict → context-sensitive: [SEQ]=naviga step; [KEYS]=cambia ottava
44. [KEYS] in modalità non-PLAY → [KEYS] valido solo in PLAY mode
45. MIXER POT2=PAN ridondante → ridefinito in batch 7 (vedi)
46. MIXER POT3/POT4 → ridefiniti in batch 7 (vedi)
47. ENC1 click non definito fuori da PLAY → reset parametro evidenziato al default
48. NOTE/MONO POT4 → TRANSPOSE per traccia (±12 semitoni)
49. Velocity lane POT durante SHIFT → tutti pot disabilitati eccetto POT3 = velocity value
50. SOLO toggle e multi-solo → toggle; multi-solo possibile
51. SAMPLE editor doppio accesso incoerente → SHIFT in SOUND = sempre Envelope per tutti i tipi
52. NOTE/ARP scala di default → C Major

### Batch 7 — "Check finale"
53. POT3=LOOP MODE è selezione discreta su pot → ENC2 click in SOUND MELODIC = cicla loop mode; POT3 = LOOP START (0–100%) → poi ridefinito in batch 8
54. QUANTIZE su pot sbagliato → ENC1 click in NOTE/MONO = QUANTIZE → poi ridefinito in batch 8
55. ARP PATTERN su pot sbagliato → ENC2 rotate in NOTE/ARP = cicla PATTERN; POT1 = VELOCITY DEFAULT → poi confermato in batch 8
56. NOTE mode "parametro evidenziato" mai definito → semplificato: ENC2 rotate in NOTE = sempre il primario discreto
57. MIXER sends vs FX sends (sends in due posti) → rimuovi sends da MIXER; MIXER = VOLUME/PAN/HIGH EQ/LOW EQ
58. SAMPLE editor pagina 2 su DRUM → ENC2 click in SAMPLE DRUM = nessun ciclo (solo pagina 1)
59. SHIFT held in FX senza azione → SHIFT held in FX = secondo layer parametri; ENC2 click libero

### Batch 8 — Conflitti da soluzioni batch 7
60. SOUND MELODIC ENC2 click conflict (toggle tipo vs cicla loop mode) → ENC2 click = toggle DRUM/MELODIC; SHIFT+ENC2 click = cicla loop mode
61. NOTE/MONO ENC1 click conflict (QUANTIZE vs reset parametro globale) → QUANTIZE su SHIFT+ENC2 click; ENC1 click = reset ottava (regola globale)
62. NOTE/MONO POT1 ridondante con ENC2 rotate (entrambi SCALE) → POT1 = VELOCITY DEFAULT; ENC2 = SCALE
63. NOTE/CHORD POT1 ridondante con ENC2 rotate (entrambi TYPE) → POT1 = VELOCITY DEFAULT; ENC2 = TYPE
64. FX ENC2 click libero → ENC2 click in FX = randomize parametri effetto (POT2/3/4)
65. NOTE/MONO POT3 senza funzione → POT3 = HUMANIZE (variazione casuale velocity)
66. SOUND MELODIC nessun preview (ENC2 click occupato) → accettato: preview solo da SAMPLE editor

### Batch 9 — "Controlla bene che siano gli ultimi"
67. GATE LENGTH in PATTERN vs GATE in NOTE/ARP (priorità?) → NOTE/ARP GATE ha priorità; PATTERN GATE LENGTH = fallback; OLED mostra `[ARP]`

### Batch 10 — "Fai un'ultima verifica"
68. SHIFT+[PLAY/STOP] durante velocity lane → OVERVIEW ha sempre priorità assoluta
69. FX SEND p-lock: quale effetto? → sempre l'effetto assegnato alla traccia; tooltip OLED `FX:[nome]`

### Batch 11 — "Controlla bene se sono davvero gli ultimi"
70. Auto p-lock durante Envelope sub-view (SOUND + SHIFT held) → pausa registrazione p-lock; OLED `[REC PAUSED]`

### Batch 12 — Toggle SEQ/KEYS senza controllo dedicato
71. `[SEQ]`/`[KEYS]` non avevano un controllo hardware assegnato per il toggle → **FB2** ridefinita da "REC shortcut" (ridondante con ENC2 click) a toggle `[SEQ]` ↔ `[KEYS]`; REC resta unico su ENC2 click in tutte le modalità

### Batch 13 — Audit sistematico encoder/FB per modalità
72. Riga fantasma in tabella SOUND/MELODIC (`SHIFT + ENC2 click (in FX)` duplicava la definizione già presente nella schermata FX) → rimossa, era un residuo di editing
73. SOUND/SAMPLE: ENC2 click occupato da "cicla pagine" rende irraggiungibile il toggle DRUM↔MELODIC → **limite accettato**: da SAMPLE si torna prima a DRUM/MELODIC (ENC1 rotate) per cambiare tipo
74. REC (ENC2 click) irraggiungibile in SOUND e FX, dove ENC2 click è occupato da toggle-tipo/randomize → aggiunto **SHIFT + ENC1 click = REC** in tutte le sotto-schermate di SOUND (DRUM/MELODIC/SAMPLE) e in FX
75. Ambiguità "[KEYS] valido solo in PLAY mode" (era poco chiaro se schermata o stato transport) → chiarito: **solo nella schermata OVERVIEW/PLAY**; rimossa la riga `[KEYS]` da NOTE/MONO (era in contraddizione con la regola)
76. Voce storica #8 ("REC funziona solo in PLAY e SOUND mode") risultava superata dai batch successivi (in SOUND, REC non è mai stato su ENC2 click) → voce di log obsoleta, le tabelle finali per schermata sono l'unica fonte autoritativa

### Batch 14 — REC spostato su FB2, ENC2 click sempre contestuale
77. `ENC2 click` come portatore globale di REC creava eccezioni continue (SOUND, FX) e mescolava azioni globali con azioni contestuali → **REC spostato su FB2** (bottone fisico dedicato, sempre raggiungibile, nessuna eccezione, simmetrico a FB1=PLAY/STOP)
78. Toggle `[SEQ]`/`[KEYS]` (batch 12, su FB2 semplice) → spostato su **SHIFT + FB2**, liberando FB2 per REC
79. Patch batch 13 #74 (`SHIFT + ENC1 click = REC` in SOUND×3 e FX) → **rimossa**, non più necessaria: REC è ora sempre su FB2 indipendentemente dalla schermata
80. `ENC2 click` liberato in 6 schermate dove era REC → ridefinito con azioni contestuali all'OLED:
    - OVERVIEW/PLAY: toggle step sotto il cursore statico
    - PATTERN: aggiungi/rimuovi pattern corrente dalla chain
    - NOTE/MONO: preview nota corrente
    - NOTE/CHORD: preview accordo corrente
    - NOTE/ARP: preview pattern arpeggio corrente
    - MIXER: cicla parametro bersaglio di ENC2 rotate (risolve anche l'ambiguità mai chiarita di batch 4 #27 su "quale parametro selezionato")

**Risultato architetturale:** `ENC1`/`ENC2` (click e SHIFT+click) sono ora **sempre e solo azioni contestuali alla schermata mostrata sull'OLED**; le uniche azioni globali sono su **FB1** (PLAY/STOP, SHIFT=OVERVIEW) e **FB2** (REC, SHIFT=SEQ/KEYS toggle). FB3–FB8 restano la navigazione diretta tra modalità.

---

## Problemi Accettabili — Lasciati irrisolti

A. Loop mode change senza feedback visivo durante Envelope sub-view in SOUND/MELODIC — l'utente è consapevole di essere in sub-view.

B. Naming "CHORD" — ARP pattern "CHORD" vs sub-schermata NOTE/CHORD — il contesto disambigua sempre.
