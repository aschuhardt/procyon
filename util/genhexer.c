/*
 * GenHexer
 * Small utility for converting files to C headers
 */

#include <stdio.h>

int main(int argc, const char** argv) {
  if (argc < 4) {
    fputs("Usage: genhexer <infile> <outfile> <name>\n", stderr);
    return -1;
  }

  const char* in_path = argv[1];
  const char* out_path = argv[2];
  const char* name = argv[3];

  FILE* in_file = fopen(in_path, "rb");
  FILE* out_file = fopen(out_path, "wb+");

  // get input file size
  fseek(in_file, 0, SEEK_END);
  const unsigned long in_size = ftell(in_file);

  // write include-guards and declarations
  fprintf(out_file, "#ifndef %s_H\n", name);
  fprintf(out_file, "#define %s_H\n", name);

  // +1 to length to account for null terminator
  fprintf(out_file, "const unsigned char %s[%lu] = { ", name, in_size + 1);

  // allocate a buffer on the stack,
  unsigned char buffer[4096];
  const size_t buffer_len = sizeof(buffer) / sizeof(unsigned char);

  // reset cursor position for input file and perform buffered reading
  fseek(in_file, 0, SEEK_SET);
  size_t read_n = 0;
  do {
    read_n = fread(&buffer, sizeof(unsigned char), buffer_len, in_file);
    for (unsigned long i = 0; i < read_n; ++i) {
      // write each byte from the file as a hexadecimal-formatted literal
      // followed by a comma
      fprintf(out_file, "0x%02x, ", buffer[i]);
    }
  } while (read_n == buffer_len);

  // tack a null-terminator onto the end for good measure
  fputs("0x00 };\n", out_file);

  // terminate include guard
  fprintf(out_file, "#endif\n");

  fclose(in_file);
  fclose(out_file);

  // report results
  printf(
      "GenHexer input: %s,\n"
      "         output: %s,\n"
      "         name: %s\n"
      "         size: %lu bytes\n",
      in_path, out_path, name, in_size);
}
