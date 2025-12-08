| Start   | End     | Size | description                             |
| ------- | ------- | ---- | --------------------------------------- |
| 0x9f000 | 0x9f000 | 1    | keycode status (Extended byte) bool     |
| 0x9f001 | 0x9f001 | 1    | keyboard modifier keys                  |
| 0x9f002 | 0x9f002 | 1    | Keycode register                        |
| 0x9f003 | 0x9f003 | 1    | ASCII input register                    |
| 0x9f100 | 0x9f1ff | 256  | Scancode set                            |
| 0x9f200 | 0x9f3ff | 256  | Keycode keyboard layout (256-511 > MAJ) |
| 0x9f400 | 0x9f4ff | 256  | Keycode buffer                          |
| 0x9f500 | 0x9f5ff | 256  | ASCII input buffer                      |
