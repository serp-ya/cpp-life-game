#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

const std::string file_name = "data.txt";
const int MAX_INIT_CELLS_COUNT = 1000;

const int CRITICAL_COUNT_OF_GENERATIONS = 1000;
  
const std::string CELLS_DEAD_MSG = "All cells are dead. Game over";
const std::string WORLD_STAGNATED_MSG = "The world has stagnated. Game over";
  
const char NULL_CHAR = static_cast<char>(0);
const char DEAD_CELL = '-';
const char LIVE_CELL = '*'; 

// get number from file
int get_number(std::ifstream& file_data) {
  std::string buffer_string = "";
  
  file_data >> buffer_string;

  try {
    return std::stoi(buffer_string);
  } catch(std::invalid_argument error) {
    std::cout << "Error by geting number from string: " << buffer_string << std::endl;
    std::cout << error.what() << std::endl;
    return 0;
  }
}

// clean incoming data memory
void delete_data(int** data, int size) {
  for(int i = 0; i < size; i++) {
    delete[] data[i];
    data[i] = nullptr;
  }
  delete data;
}

// create canvas (rows x cols) size
char** create_canvas(int rows, int cols) {
  char** canvas = new char*[rows];

  for(int r = 0; r < rows; r++) {
    canvas[r] = new char[cols];
    
    for(int c = 0; c < cols; c++) {
      canvas[r][c] = DEAD_CELL;
    }
  }

  return canvas;
}

// init canvas by default and provided data
void init_canvas(char** canvas, int rows, int cols, int** data, int cells) {
  for(int i = 0; i < cells; i++) {
    const int r = data[i][0];
    const int c = data[i][1];

    canvas[r][c] = LIVE_CELL;
  }
}

// just print canvas
void print_canvas(char** canvas, int rows, int cols) {
  for(int r = 0; r < rows; r++) {
    for(int c = 0; c < cols; c++) {
      std::cout << canvas[r][c];
      if (c < cols - 1) {
        // without trailing space
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
}

// just delete canvas
void delete_canvas(char** canvas, int rows) {
  for(int r = 0; r < rows; r++) {
    delete[] canvas[r];
    canvas[r] = nullptr;
  }

  delete[] canvas;
}

// safety get cell value
// r - current row
// c - current column
char get_cell(char** canvas, int r, int c, int rows, int cols) {
  if (r < 0 || r >= rows || c < 0 || c >= cols) {
    return NULL_CHAR;
  }
  return canvas[r][c];
}

// get count of the cell's live neighbours
// r - current row
// c - current column
int get_live_neighbours(char** canvas, int r, int c, int rows, int cols) {
  int live_neighbours = 0;

  // all top neighbours
  get_cell(canvas, r-1, c, rows, cols) == LIVE_CELL && ++live_neighbours;
  get_cell(canvas, r-1, c-1, rows, cols) == LIVE_CELL && ++live_neighbours;
  get_cell(canvas, r-1, c+1, rows, cols) == LIVE_CELL && ++live_neighbours;
  
  // all bottom neighbours
  get_cell(canvas, r+1, c, rows, cols) == LIVE_CELL && ++live_neighbours;
  get_cell(canvas, r+1, c-1, rows, cols) == LIVE_CELL && ++live_neighbours;
  get_cell(canvas, r+1, c+1, rows, cols) == LIVE_CELL && ++live_neighbours;

  // left and right neighbours
  get_cell(canvas, r, c-1, rows, cols) == LIVE_CELL && ++live_neighbours;
  get_cell(canvas, r, c+1, rows, cols) == LIVE_CELL && ++live_neighbours;

  return live_neighbours;
}

// check this cell should revive
bool should_revive_cell(char cell, int live_neighbours) {
  return cell == DEAD_CELL && live_neighbours == 3;
}

// check this cell should dead
bool should_kill_cell(char cell, int live_neighbours) {
  return cell == LIVE_CELL && (live_neighbours < 2 || live_neighbours > 3);
}

// create new generation
char** update_generation(char** canvas, int rows, int cols) {
  char** new_canvas = create_canvas(rows, cols);

  for(int r = 0; r < rows; r++) {
    for(int c = 0; c < cols; c++) {
      const char current_cell = canvas[r][c];
      const int live_neighbours = get_live_neighbours(canvas, r, c, rows, cols);

      if (should_revive_cell(current_cell, live_neighbours)) {
        new_canvas[r][c] = LIVE_CELL;
      } else if (should_kill_cell(current_cell, live_neighbours)) {
        new_canvas[r][c] = DEAD_CELL;
      } else {
        new_canvas[r][c] = current_cell;
      }
    }
  }

  return new_canvas;
}

// just print generation info message
void print_generation_info(int generation, int alive_cells) {
  std::cout << "Generation: " << generation << ". Alive cells: " << alive_cells << std::endl;
}

// count all live cells on the canvas
int count_alive_cells(char** canvas, int rows, int cols) {
  int live_cells = 0;

  for(int r = 0; r < rows; r++) {
    for(int c = 0; c < cols; c++) {
      if (canvas[r][c] == LIVE_CELL) {
        ++live_cells;
      }
    }
  }

  return live_cells;
}

// should compare two canvases and return true if they are different
bool get_canvas_has_changes(char** canvas_a, char** canvas_b, int rows, int cols) {
  for(int r = 0; r < rows; r++) {
    for(int c = 0; c < cols; c++) {
      if (canvas_a[r][c] != canvas_b[r][c]) {
        return true;
      }
    }
  }

  return false;
}

// game core function
void core(char** canvas, int rows, int cols, int init_cells) {
  char** new_canvas;
  int generation = 1;

  // first render
  print_canvas(canvas, rows, cols);
  print_generation_info(generation, init_cells);
  std::cout << std::endl;
  sleep(1);

  while(++generation < CRITICAL_COUNT_OF_GENERATIONS) {
    new_canvas = update_generation(canvas, rows, cols);

    const bool has_changes = get_canvas_has_changes(canvas, new_canvas, rows, cols);
    const int alive_cells = count_alive_cells(new_canvas, rows, cols);

    delete_canvas(canvas, rows);
    canvas = nullptr;

    std::system("clear");
    print_canvas(new_canvas, rows, cols);
    print_generation_info(generation, alive_cells);

    if (!has_changes || alive_cells == 0) {
      const std::string message = has_changes ? CELLS_DEAD_MSG : WORLD_STAGNATED_MSG;

      std::cout << message << std::endl;

      break;
    }

    canvas = new_canvas;
    std::cout << std::endl;
    sleep(1);
  }

  delete_canvas(new_canvas, rows);
  new_canvas = nullptr;
}
  
int main() {
  std::system("clear");
  std::ifstream fs_data(static_cast<std::string>("./") + file_name);

  if (!fs_data.is_open()) {
    std::cout << "[ERROR]: File (" << file_name << ") not found!" << std::endl;
    fs_data.close();
    return -1;
  }

  const int rows = get_number(fs_data);
  const int cols = get_number(fs_data);
  
  int init_cells_count = 0;
  int** init_cells = new int*[MAX_INIT_CELLS_COUNT];

  while(!fs_data.eof()) {
    if (init_cells_count >= MAX_INIT_CELLS_COUNT) {
      break;
    }
    const int x = get_number(fs_data);
    const int y = get_number(fs_data);

    init_cells[init_cells_count] = new int[2];
    init_cells[init_cells_count][0] = x;
    init_cells[init_cells_count][1] = y;
    
    ++init_cells_count;
  }

  fs_data.close();

  char** canvas = create_canvas(rows, cols);
  init_canvas(canvas, rows, cols, init_cells, init_cells_count);

  core(canvas, rows, cols, init_cells_count);

  delete_data(init_cells, init_cells_count);
  init_cells = nullptr;
  canvas = nullptr;

  return 0;
}
