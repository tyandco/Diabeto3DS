# Diabeto 3DS

Offline 3DS homebrew prototype for Diabeto.

## Current MVP

- Dashboard on the top screen.
- Touch/button navigation on the bottom screen.
- Local risk score calculator.
- Daily log fields.
- Offline variable tips.
- Save/load profile and log data from `sdmc:/3ds/Diabeto3DS/save.bin`.

## Build

Install devkitPro with the 3DS toolchain, then run:

```sh
make
```

Output:

```text
Diabeto3DS.3dsx
Diabeto3DS.smdh
```

The `.cia` target is intentionally stubbed until final icon/banner assets are ready.

## Controls

- `A`: open daily log
- `X`: open profile/risk setup
- `Y`: save
- `START`: exit
- `B`: back to dashboard
- `D-Pad`: change selected field
- Touch bottom buttons for main navigation

