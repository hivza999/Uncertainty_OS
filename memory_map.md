| Start   | End     | Size  | description                             |
| ------- | ------- | ----- | --------------------------------------- |
| 0x00500 | 0x044ff | 16384 | base kernel                             |
| 0x80000 | ?       | ?     | Memory map                              |
| 0x90000 | 0x900ff | 256   | Registers                               |
| 0x90000 | 0x90000 | 1     | keycode status (Extended byte) bool     |
| 0x90001 | 0x90001 | 1     | keyboard modifier keys                  |
| 0x90002 | 0x90002 | 1     | Keycode register                        |
| 0x90003 | 0x90003 | 1     | ASCII input register                    |
| 0x90100 | 0x901ff | 256   | Scancode set                            |
| 0x90200 | 0x903ff | 256   | Keycode keyboard layout (256-511 > MAJ) |
| 0x90400 | 0x904ff | 256   | Keycode buffer                          |
| 0x90500 | 0x905ff | 256   | ASCII input buffer                      |
