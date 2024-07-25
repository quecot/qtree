#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum node_type { NODE_FILE, NODE_DIRECTORY };
enum output_format { OUTPUT_TEXT, OUTPUT_JSON, OUTPUT_XML };

struct DirEntry {
  char *name;
  enum node_type type;
  struct DirEntry **children;
  int children_count;
};

void traverse_dir(const char *path, struct DirEntry *root);
void print_text(struct DirEntry *root, int indent, FILE *out);
void print_json(struct DirEntry *root, FILE *out);
void print_xml(struct DirEntry *root, FILE *out);
char *escape_xml(const char *str);
void free_dir_tree(struct DirEntry *root);
void print_help(const char *prog_name);
void print_version(void);

int main(int argc, char **argv) {
  int help_flag                       = 0;
  int version_flag                    = 0;
  char *output_file                   = NULL;
  int output_format                   = OUTPUT_TEXT;

  int json_flag                       = 0;
  int xml_flag                        = 0;

  static struct option long_options[] = {
      {"json", no_argument, 0, 'j'},         {"xml", no_argument, 0, 'x'},
      {"help", no_argument, 0, 'h'},         {"version", no_argument, 0, 'v'},
      {"output", required_argument, 0, 'o'}, {0, 0, 0, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "jxhvo:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'j':
      if (json_flag || xml_flag) {
        puts("Error: Incompatible flags --json and --xml or duplicate flags.");
        return 1;
      }
      json_flag     = 1;
      output_format = OUTPUT_JSON;
      break;
    case 'x':
      if (json_flag || xml_flag) {
        puts("Error: Incompatible flags --json and --xml or duplicate flags.");
        return 1;
      }
      xml_flag      = 1;
      output_format = OUTPUT_XML;
      break;
    case 'h':
      if (help_flag || version_flag || json_flag || xml_flag) {
        puts("Error: Incompatible flags.");
        return 1;
      }
      help_flag = 1;
      break;
    case 'v':
      if (version_flag || help_flag || json_flag || xml_flag) {
        puts("Error: Incompatible flags.");
        return 1;
      }
      version_flag = 1;
      break;
    case 'o':
      output_file = optarg;
      break;
    default:
      print_help(argv[0]);
      return 1;
    }
  }

  if (version_flag) {
    print_version();
    return 0;
  }

  if (help_flag) {
    print_help(argv[0]);
    return 0;
  }

  if (optind != argc - 1) {
    print_help(argv[0]);
    return 1;
  }

  char *path            = argv[optind];

  struct DirEntry *root = malloc(sizeof(struct DirEntry));
  if (!root) {
    perror("malloc");
    return 1;
  }
  root->name = strdup(path);
  if (!root->name) {
    perror("strdup");
    free(root);
    return 1;
  }
  root->type           = NODE_DIRECTORY;
  root->children       = NULL;
  root->children_count = 0;

  traverse_dir(path, root);

  FILE *out = stdout;
  if (output_file) {
    out = fopen(output_file, "w");
    if (!out) {
      perror("fopen");
      free_dir_tree(root);
      return 1;
    }
  }

  char buffer[8192];
  size_t offset = 0;
  buffer[0]     = '\0';

  if (output_format == OUTPUT_JSON) {
    print_json(root, out);
  } else if (output_format == OUTPUT_XML) {
    print_xml(root, out);
  } else {
    print_text(root, 0, out);
  }

  if (output_format != OUTPUT_TEXT) {
    fwrite(buffer, 1, offset, out);
  }

  if (out != stdout) {
    fclose(out);
  }

  free_dir_tree(root);

  return 0;
}

void traverse_dir(const char *path, struct DirEntry *root) {
  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    struct DirEntry *child = malloc(sizeof(struct DirEntry));
    if (!child) {
      perror("malloc");
      closedir(dir);
      return;
    }
    child->name = strdup(entry->d_name);
    if (!child->name) {
      perror("strdup");
      free(child);
      closedir(dir);
      return;
    }
    child->type           = (entry->d_type == DT_DIR) ? NODE_DIRECTORY : NODE_FILE;
    child->children       = NULL;
    child->children_count = 0;

    if (entry->d_type == DT_DIR) {
      char subdir_path[1024];
      snprintf(subdir_path, sizeof(subdir_path), "%s/%s", path, entry->d_name);
      traverse_dir(subdir_path, child);
    }

    root->children =
        realloc(root->children, (root->children_count + 1) * sizeof(struct DirEntry *));
    if (!root->children) {
      perror("realloc");
      free(child->name);
      free(child);
      closedir(dir);
      return;
    }
    root->children[root->children_count++] = child;
  }

  closedir(dir);
}

void print_text(struct DirEntry *root, int indent, FILE *out) {
  char buffer[1024];
  int n;

  if (root->type == NODE_DIRECTORY) {
    n = snprintf(buffer, sizeof(buffer), "%*s%s/\n", indent, "", root->name);
    fwrite(buffer, 1, n, out);
    for (int i = 0; i < root->children_count; i++) {
      print_text(root->children[i], indent + 2, out);
    }
  } else {
    n = snprintf(buffer, sizeof(buffer), "%*s%s\n", indent, "", root->name);
    fwrite(buffer, 1, n, out);
  }
}

void print_json(struct DirEntry *root, FILE *out) {
  char buffer[1024];

  if (root->type == NODE_DIRECTORY) {
    sprintf(buffer, "{\"name\":\"%s\",\"type\":\"directory\",\"children\":", root->name);
    fwrite(buffer, strlen(buffer), 1, out);
    if (root->children_count > 0) {
      sprintf(buffer, "[");
      fwrite(buffer, strlen(buffer), 1, out);
      for (int i = 0; i < root->children_count; i++) {
        if (i > 0) {
          sprintf(buffer, ",");
          fwrite(buffer, strlen(buffer), 1, out);
        }
        print_json(root->children[i], out);
      }
      sprintf(buffer, "]");
      fwrite(buffer, strlen(buffer), 1, out);
    } else {
      sprintf(buffer, "[]");
      fwrite(buffer, strlen(buffer), 1, out);
    }
    sprintf(buffer, "}");
    fwrite(buffer, strlen(buffer), 1, out);
  } else {
    sprintf(buffer, "{\"name\":\"%s\",\"type\":\"file\"}", root->name);
    fwrite(buffer, strlen(buffer), 1, out);
  }
}

void print_xml(struct DirEntry *root, FILE *out) {
  char buffer[1024];

  if (root->type == NODE_DIRECTORY) {
    sprintf(buffer, "<directory name=\"%s\">", escape_xml(root->name));
    fwrite(buffer, strlen(buffer), 1, out);
    for (int i = 0; i < root->children_count; i++) {
      print_xml(root->children[i], out);
    }
    sprintf(buffer, "</directory>");
    fwrite(buffer, strlen(buffer), 1, out);
  } else {
    sprintf(buffer, "<file name=\"%s\"/>", escape_xml(root->name));
    fwrite(buffer, strlen(buffer), 1, out);
  }
}

char *escape_xml(const char *str) {
  static char buffer[1024];
  char *p = buffer;
  while (*str) {
    switch (*str) {
    case '<':
      *p++ = '&';
      *p++ = 'l';
      *p++ = 't';
      *p++ = ';';
      break;
    case '>':
      *p++ = '&';
      *p++ = 'g';
      *p++ = 't';
      *p++ = ';';
      break;
    case '&':
      *p++ = '&';
      *p++ = 'a';
      *p++ = 'm';
      *p++ = 'p';
      *p++ = ';';
      break;
    case '"':
      *p++ = '&';
      *p++ = 'q';
      *p++ = 'u';
      *p++ = 'o';
      *p++ = 't';
      *p++ = ';';
      break;
    default:
      *p++ = *str;
      break;
    }
    str++;
  }
  *p = '\0';
  return buffer;
}

void free_dir_tree(struct DirEntry *root) {
  if (root->type == NODE_DIRECTORY) {
    for (int i = 0; i < root->children_count; i++) {
      free_dir_tree(root->children[i]);
    }
  }
  free(root->name);
  free(root->children);
  free(root);
}

void print_help(const char *prog_name) {
  printf("Usage: %s [options] <directory>\n", prog_name);
  printf("Options:\n");
  printf("  -j, --json      Output as JSON\n");
  printf("  -x, --xml       Output as XML\n");
  printf("  -h, --help      Show this help message\n");
  printf("  -v, --version   Show version information\n");
  printf("  -o, --output    Specify output file\n");
}

void print_version(void) { puts("qtree 0.0.1"); }
