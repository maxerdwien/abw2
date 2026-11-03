# Running on SteamOS (Steam Deck)

The game builds as a native Linux binary. SteamOS has no compiler and a read-only
system partition, so the build runs inside a temporary Ubuntu container using
`podman`, which ships with SteamOS. Nothing gets installed onto the system itself.

## 1. Switch to Desktop Mode

Steam button → **Power** → **Switch to Desktop**. Open **Konsole** (the terminal)
from the application launcher.

Check that podman is available:

```sh
podman --version
```

It ships with SteamOS 3.5 and later. On an older SteamOS, update the system first.

## 2. Get the code

```sh
cd ~
git clone https://github.com/maxerdwien/abw2.git
cd abw2
```

If the repository is private, sign in first (for example `gh auth login`, or use an
SSH remote).

## 3. Build

```sh
./Project1/build-steamos.sh
```

The first build downloads the Ubuntu image and compilers, so it takes a few minutes
and needs internet. Later builds are faster. When it finishes it prints:

```
built /home/deck/abw2/Project1/linux/acsf
```

The output is:

| Path | What it is |
| --- | --- |
| `Project1/linux/acsf` | the game |
| `Project1/linux/lib/` | SDL2 add-on libraries SteamOS doesn't include (found automatically) |

The game loads its art and sounds from `Project1/Project1/assets/`, relative to the
executable. **Don't move `acsf` out of `Project1/linux/`.** If you want the game
somewhere else, move the whole `abw2` folder.

To delete the build output: `./Project1/build-steamos.sh clean`.

## 4. Try it in Desktop Mode

```sh
./Project1/linux/acsf
```

It starts fullscreen. Press **Esc** to drop to a window, or **Q** to quit.

## 5. Add it to Steam for Game Mode

1. Open Steam in Desktop Mode.
2. **Games** → **Add a Non-Steam Game to My Library…** → **Browse…**
3. Change the file-type filter to **All Files**, select
   `~/abw2/Project1/linux/acsf`, and click **Add Selected Programs**.
4. Optional: in the game's **Properties**, rename the shortcut (for example to
   "Alaskan Cosmobear Spacefighting").
5. Return to Game Mode using the **Return to Gaming Mode** icon on the desktop. The
   game appears under **Library → Non-Steam**.

Steam Input presents the Deck's built-in controls as a standard gamepad, so the
default **Gamepad** controller layout works. External controllers (Bluetooth or USB)
also work, as does a GameCube controller adapter (WUP-028).

## Controls

| Input | Action |
| --- | --- |
| Back / View button | Quit |
| X (character select) | Add an AI player to an empty slot, or clear AI players |
| LB (character select) | Switch your slot between human and AI |
| F (keyboard) | Toggle fullscreen |
| Esc (keyboard) | Leave fullscreen |
| M (keyboard) | Toggle sound |
| Q (keyboard) | Quit |

## Updating

```sh
cd ~/abw2
git pull
./Project1/build-steamos.sh
```

The Steam shortcut keeps working because the binary stays at the same path.

## Troubleshooting

- **`error while loading shared libraries`**: the `linux/lib` folder is missing or
  incomplete. Run `./Project1/build-steamos.sh` again.
- **Black screen or missing graphics**: `acsf` was moved or copied out of
  `Project1/linux/`. Run it from there, or move the whole repository instead.
- **No sound**: sound can only be toggled from a keyboard (**M**), so check that a
  keyboard press didn't mute it. Release builds start with sound on. Only Visual
  Studio *Debug* builds start muted.
- **The build script can't pull the image**: check your internet connection. If
  podman complains about its storage, run `podman system reset` and try again.
  That command deletes all local podman containers and images.
