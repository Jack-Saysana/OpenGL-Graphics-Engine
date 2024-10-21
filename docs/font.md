# Fonts

The engine also supports font importing capabilities for usage during text rendering of UI components. However, the engine is unable to import raw .ttf files. Instead, the engine is able to import an intermediary file format which can be created utilizing [this tool](https://github.com/Jack-Saysana/Font-Importer).

## Functions

### `int import_font(char *bin_path, char *tex_path, F_GLYPH **dest)`

**Arguments**

- `char *bin_path`: The path to the binary font file

- `char *tex_path`: The path to the png texture to be used by the font

- `F_GLYPH **dest`: The location where the font should be allocated at. A double pointer is used here, since to the engine, a font is simply an array of `F_GLYPH` structs. The populated array can then be used anywhere fonts are required in the engine.

**Returns**

0 if successful, -1 if an error occured
