#include "glade/exception/GladeException.h"

#include "glade/util/CSVReader.h"
#include "glade/debug/log.h"
#include "glade/exception/GladeException.h"

using namespace std;

const char DELIMITER = ',';

void CSVReader::read(istream &input, vector<vector<string>> &output) {
  if (!input.good()) {
    throw GladeException("CSVReader: input stream is broken");
  }

  char ch = 0, previous_char = 0, next_char = 0;
  bool inside_quoted_field = false;
  string field;
  vector<string> row;

  output.clear();

  input >> noskipws;

  while (input >> ch, !input.eof()) {
    next_char = input.peek();

    switch (ch) {
    case ::DELIMITER:

      if (inside_quoted_field) {
        field.append(1, ch);
      } else {
        row.push_back(field);
        field.clear();
      }

      break;
    case '\n':
      row.push_back(field);
      field.clear();
      output.push_back(row);
      row.clear();
      break;
    case '\r':
      // skip
      break;
    case '"':

      if (previous_char == ::DELIMITER || previous_char == 0) {
        inside_quoted_field = true;
      } else if (inside_quoted_field) {
        if (next_char == '"') {
          field.append(1, ch);
          input.get();
        } else {
          if (next_char == ::DELIMITER || next_char == EOF || next_char == '\n') {
            inside_quoted_field = false;
          } else {
            throw GladeException("Corrupted CSV");
          }
        }
      } else {
        field.append(1, ch);
      }

      break;
    default:
      field.append(1, ch);
      break;
    }

    previous_char = ch;
  }

  if (!field.empty()) {
    row.push_back(field);
  }

  if (!row.empty()) {
    output.push_back(row);
  }
}

void CSVReader::read(vector<char> &input, vector<vector<string>> &output) {
  char ch = 0, previous_char = 0, next_char = 0;
  bool inside_quoted_field = false;
  string field;
  vector<string> row;

  output.clear();

  for (int i = 0; i < input.size(); ++i) {
    ch = input[i];
    next_char = input[i + 1];

    switch (ch) {
    case ::DELIMITER:

      if (inside_quoted_field) {
        field.append(1, ch);
      } else {
        row.push_back(field);
        field.clear();
      }

      break;
    case '\n':
      row.push_back(field);
      field.clear();
      output.push_back(row);
      row.clear();
      break;
    case '\r':
      // skip
      break;
    case '"':

      if (previous_char == ::DELIMITER || previous_char == 0) {
        inside_quoted_field = true;
      } else if (inside_quoted_field) {
        if (next_char == '"') {
          field.append(1, ch);
          ++i;
        } else {
          if (next_char == ::DELIMITER || next_char == EOF || next_char == '\n') {
            inside_quoted_field = false;
          } else {
            throw GladeException("Corrupted CSV");
          }
        }
      } else {
        field.append(1, ch);
      }

      break;
    default:
      field.append(1, ch);
      break;
    }

    previous_char = ch;
  }

  if (!field.empty()) {
    row.push_back(field);
  }

  if (!row.empty()) {
    output.push_back(row);
  }
}
