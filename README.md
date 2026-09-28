# Earth 106 — VST3 (Windows x64) by NACHE  ·  v1.1

Synthé soustractif inspiré du Juno-106 (2 oscillateurs PolyBLEP + sub + bruit, ADSR par oscillateur, chorus I / II / I+II, delay),
avec un filtre central « globe » à caractère MS-20 (12 dB, saturation, résonance jusqu'à l'auto-oscillation, LOW CUT / HIGH CUT).

## Compiler avec GitHub (rien à installer)
1. Crée un dépôt GitHub et pousse le contenu de ce dossier (branche `main`).
2. Onglet **Actions** → « Build Earth 106 VST3 (Windows) » (se lance seul au push, ou via *Run workflow*).
3. À la fin, télécharge l'artefact **Earth106-VST3-Windows** (zip).
4. Décompresse et copie le dossier `Earth 106.vst3` dans `C:\Program Files\Common Files\VST3\`, puis rescane les plugins dans ton DAW.

Pour une release publique : `git tag v1.1 && git push --tags` (le zip est attaché à la release).

## Notes
* Runtime C++ statique : pas de Visual C++ Redistributable à installer.
* Presets utilisateur : `%APPDATA%\NACHE\Earth106\user_presets.xml`.
* Police « by NACHE » : New Rocker (OFL) téléchargée automatiquement par le workflow (sinon police système de secours).
* Licence : JUCE 8 est en AGPLv3 / licence commerciale, et le SDK VST3 est en GPLv3 / licence Steinberg. Un dépôt public sous AGPLv3 est le plus simple ; pour un plugin fermé, il faut les licences commerciales.
