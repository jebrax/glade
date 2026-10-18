#include <map>

#include "glade/util/WavefrontObjReader.h"
#include "glade/render/meshes/Mesh.h"

static std::vector<char>::const_iterator read_pos;
static int position_inside_the_line;

static std::vector<std::vector<float> *> positions;
static std::vector<std::vector<float> *> normals;
static std::vector<std::vector<float> *> tex_coords;
static std::vector<std::string> faces;
static std::map<std::string, int> vertex_map;

static bool get_next_line(const std::vector<char> &in, std::string &line) {
  line.clear();

  for (; ::read_pos != in.end(); ++::read_pos) {
    if (*::read_pos == '\r' || *::read_pos == '\n') {
      if (line.size() > 0) {
        return true;
      }

      continue;
    }

    line += *::read_pos;
  }

  return (bool)line.size();
}

static bool get_next_token(std::string &line, std::string &token, const char separator = ' ') {
  token.clear();

  for (; ::position_inside_the_line < line.size(); ++::position_inside_the_line) {
    if (line[::position_inside_the_line] == separator) {
      if (token.size() > 0) {
        return true;
      }

      continue;
    }

    token += line[::position_inside_the_line];
  }

  return (bool)token.size();
}

static void parse_vertex_position(std::string &line) {
  std::string token;
  std::vector<float> *position = new std::vector<float>;

  for (int i = 1; i <= 3; ++i) {
    ::get_next_token(line, token);
    position->push_back(std::stof(token));
  }

  ::positions.push_back(position);
}

static void parse_vertex_normal(std::string &line) {
  std::string token;
  std::vector<float> *normal = new std::vector<float>;

  for (int i = 1; i <= 3; ++i) {
    ::get_next_token(line, token);
    normal->push_back(std::stof(token));
  }

  ::normals.push_back(normal);
}

static void parse_vertex_tex_coord(std::string &line) {
  std::string token;
  std::vector<float> *tex_coord = new std::vector<float>;

  for (int i = 1; i <= 2; ++i) {
    ::get_next_token(line, token);
    tex_coord->push_back(std::stof(token));
  }

  ::tex_coords.push_back(tex_coord);
}

bool WavefrontObjReader::putFace(std::string &face, Glade::Mesh *mesh) {
  std::string token;
  ::position_inside_the_line = 0;
  std::vector<std::string> vertices;
  ::get_next_token(face, token);

  ::get_next_token(face, token);
  vertices.push_back(token);
  ::get_next_token(face, token);
  vertices.push_back(token);
  ::get_next_token(face, token);
  vertices.push_back(token);

  // TODO: negative indices in OBJ file

  for (std::vector<std::string>::iterator vertex_string = vertices.begin();
       vertex_string != vertices.end(); ++vertex_string) {
    int position_index = 0, normal_index = 0, tex_coord_index = 0;
    ::position_inside_the_line = 0;
    ::get_next_token(*vertex_string, token, '/');
    position_index = std::stof(token);
    position_index = position_index > 0 ? position_index - 1 : position_index;

    if (::get_next_token(*vertex_string, token, '/')) {
      tex_coord_index = std::stof(token);
      tex_coord_index = tex_coord_index > 0 ? tex_coord_index - 1 : tex_coord_index;
    } else {
      // what?
    }

    if (::get_next_token(*vertex_string, token, '/')) {
      normal_index = std::stof(token);
      normal_index = normal_index > 0 ? normal_index - 1 : normal_index;
    } else {
      normal_index = position_index;
    }

    std::map<std::string, int>::iterator vertex_in_mesh = ::vertex_map.find(*vertex_string);
    int vertex_index_in_mesh;

    if (vertex_in_mesh == ::vertex_map.end()) {
      vertex_index_in_mesh = mesh->vertices.size() / 8;

      //log(">>> %f  %f  %f", positions[positionIndex]->at(0), positions[positionIndex]->at(1), positions[positionIndex]->at(2));

      if (::positions.size() <= position_index) {
        ::log("Error reading Wavefront Obj format. The vertex position index %d in the face "
              "declaration is larger than the number of faces %d",
              position_index, ::positions.size());

        return false;
      }

      mesh->vertices.push_back(::positions[position_index]->at(0));
      mesh->vertices.push_back(::positions[position_index]->at(1));
      mesh->vertices.push_back(::positions[position_index]->at(2));

      if (::normals.size() > normal_index) {
        mesh->vertices.push_back(::normals[normal_index]->at(0));
        mesh->vertices.push_back(::normals[normal_index]->at(1));
        mesh->vertices.push_back(::normals[normal_index]->at(2));
      } else {
        mesh->vertices.push_back(0.0f);
        mesh->vertices.push_back(0.0f);
        mesh->vertices.push_back(0.0f);
      }

      if (::tex_coords.size() > tex_coord_index) {
        mesh->vertices.push_back(::tex_coords[tex_coord_index]->at(0));
        mesh->vertices.push_back(::tex_coords[tex_coord_index]->at(1));
      } else {
        mesh->vertices.push_back(0.0f);
        mesh->vertices.push_back(0.0f);
      }

      ::vertex_map[*vertex_string] = vertex_index_in_mesh;
    } else {
      vertex_index_in_mesh = vertex_in_mesh->second;
    }

    mesh->indices.push_back(vertex_index_in_mesh);
  }

  return true;
}

bool WavefrontObjReader::read(const std::vector<char> &in, Glade::Mesh *mesh) {
  ::positions.clear();
  ::normals.clear();
  ::tex_coords.clear();
  ::faces.clear();
  std::string line, token;
  ::read_pos = in.begin();

  while (::get_next_line(in, line)) {
    ::position_inside_the_line = 0;
    ::get_next_token(line, token);

    if (token == "v") {
      ::parse_vertex_position(line);
    } else if (token == "vt") {
      ::parse_vertex_tex_coord(line);
    } else if (token == "vn") {
      ::parse_vertex_normal(line);
    } else if (token == "f") {
      ::faces.push_back(line);
    } else {
      continue;
    }
  }

  mesh->vertices.clear();
  mesh->indices.clear();

  // put everything into mesh

  for (std::vector<std::string>::iterator i = ::faces.begin(); i != ::faces.end(); ++i) {
    putFace(*i, mesh);
  }

  // clean up
  std::vector<std::vector<float> *>::iterator i;

  for (i = ::positions.begin(); i != ::positions.end(); ++i) {
    delete *i;
  }

  for (i = ::normals.begin(); i != ::normals.end(); ++i) {
    delete *i;
  }

  for (i = ::tex_coords.begin(); i != ::tex_coords.end(); ++i) {
    delete *i;
  }

  ::positions.clear();
  ::normals.clear();
  ::tex_coords.clear();

  ::faces.clear();
  ::vertex_map.clear();

  return true;
}
