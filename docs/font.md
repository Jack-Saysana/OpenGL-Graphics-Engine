# Fonts

The engine also supports font importing capabilities for usage during text rendering of UI components. However, the engine is unable to import raw .ttf files. Instead, the engine is able to import an intermediary file format which can be created utilizing [this tool](https://github.com/Jack-Saysana/Font-Importer).

## Functions

```int import_font(char *bin_path, char *tex_path, F_GLYPH **)```

**Arguments**

- `char *bin_path`: The path to the binary font file

- `char *tex_path`: The path to the png texture to be used by the font

- `F_GLYPH **`: The location where the font should be allocated at

**Returns**

0 if successful, -1 if an error occured
