#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

struct PrxFunction {
  std::string name;
  std::string nid;
};

struct PrxVariable {
  std::string name;
  std::string nid;
};

struct PrxLibrary {
  std::string name;
  std::string flags;
  std::vector<PrxFunction> functions;
  std::vector<PrxVariable> variables;
};

struct PrxObject {
  std::string prx;
  std::string prx_name;
  std::vector<PrxLibrary> library;
};

struct XML_SectionViewer {
private:
  std::string str;
  std::string section;
public:
  XML_SectionViewer(const std::string& str, const std::string& sect = {}) { this->str = str; section = sect; }
  void setSectionAs(const std::string& x) { section = x; }

  const std::string startHeader(void) { return "<" + section + ">"; }
  const std::string endHeader(void) { return "</" + section + ">"; }

  bool nextSection()
  {
    const std::string endHdr = endHeader();
    int eOff = str.find(endHdr);
    if (eOff != -1) {
      str = str.substr(eOff + endHdr.length());
      return true;
    }
    return false;
  }

  std::string viewSection()
  {
    const std::string startHdr = startHeader();
    const std::string endHdr = endHeader();
    int sOff = str.find(startHdr);
    int eOff = str.find(endHdr);
    if (sOff != -1 && eOff != -1) {
      sOff += startHdr.length();
      std::string view = str.substr(sOff, eOff - sOff);
      return view;
    }
    return "NULL_SECTION";
  }
};

#define ITERATE_SECTION(viewer, variable_name)                                                                            \
  for (std::string (variable_name); ((variable_name) = (viewer).viewSection()) != "NULL_SECTION"; (viewer).nextSection())

static std::vector<PrxObject> prxTable;

void compile_prx_table(const std::string& txt) {
  XML_SectionViewer viewer(txt, "PRXFILE");
  ITERATE_SECTION(viewer, x) {
    PrxObject prx_object {};
    XML_SectionViewer prx_file = XML_SectionViewer(x, "PRX");
    {
      prx_object.prx = prx_file.viewSection();
    }

    prx_file = XML_SectionViewer(x, "PRXNAME");
    {
      prx_object.prx_name = prx_file.viewSection();
    }
    
    prx_file = XML_SectionViewer(x, "LIBRARIES");
    {
      ITERATE_SECTION(prx_file, x) {      
        XML_SectionViewer libraryIterator = XML_SectionViewer(x, "LIBRARY"); {
          ITERATE_SECTION(libraryIterator, x) {
            PrxLibrary lib {};
            lib.name = XML_SectionViewer(x, "NAME").viewSection();
            lib.flags = XML_SectionViewer(x, "FLAGS").viewSection();

            for (int i = 0; i < 2; i++) {
              static const char *section_name[] = {
                "FUNCTIONS", "VARIABLES"
              };

              XML_SectionViewer NameNID_CombinationIterator(x, section_name[i]);
              ITERATE_SECTION(NameNID_CombinationIterator, x) {
                static const char *inner_section_name[] = {
                  "FUNCTION", "VARIABLE"
                };

                XML_SectionViewer NameNID_CombinationIterator2(x, inner_section_name[i]);
                ITERATE_SECTION(NameNID_CombinationIterator2, x) {              
                  std::string name, nid;
                  name = XML_SectionViewer(x, "NAME").viewSection();
                  nid = XML_SectionViewer(x, "NID").viewSection();
                  if (i == 0) {
                    lib.functions.push_back((PrxFunction) { name, nid });
                  } else if (i == 1) {
                    lib.variables.push_back((PrxVariable) { name, nid });
                  }
                }
              }
            }

            prx_object.library.push_back(lib);
          }
        }
      }
    }

    prxTable.push_back(prx_object);
  }
}

PrxObject *findPrxFile(const std::string& s) {
  for (auto& x : prxTable) {
    if (s == x.prx)
      return &x;
  }
  return nullptr;
}

bool showPrxInfoByFile(const std::string& object) {
  PrxObject *obj = findPrxFile(object);
  if (obj != nullptr) {
    printf("Found %s, ModuleName: %s\n", object.c_str(), obj->prx_name.c_str());
    printf("Found %d library\n", obj->library.size());
    for (const auto& lib : obj->library) {
      printf("Class %s\n", lib.name.c_str());
      printf("\tFunctions (%d) ->\n", lib.functions.size());
      for (const auto& funcs : lib.functions)
        printf("\t\t(%s, %s)\n", funcs.name.c_str(), funcs.nid.c_str());
      printf("\tVariables (%d) ->\n", lib.variables.size());
      for (const auto& vars : lib.variables)
        printf("\t\t%s, %s\n", vars.name.c_str(), vars.nid.c_str());
    }

    return true;
  }
  return false;
}

template<typename TypeInfo>
void showTypeInfo(const TypeInfo& t) {
  printf("%s, %s\n", t.name.c_str(), t.nid.c_str());
}

bool findTypeByName(const std::string& name) {
  for (const auto& x : prxTable) {
    for (const auto& lib : x.library) {
      for (const auto& f : lib.functions) {
        if (f.name == name) {
          printf("Function: %s, NID: %s\n", f.name.c_str(), f.nid.c_str());
          return true;
        }
      }

      for (const auto& f : lib.variables) {
        if (f.name == name) {
          printf("Variable: %s, NID: %s\n", f.name.c_str(), f.nid.c_str());
          return true;
        }
      }
    }
  }
  return false;
}

bool findTypeByNID(const std::string& nid) {
  for (const auto& x : prxTable) {
    for (const auto& lib : x.library) {
      for (const auto& f : lib.functions) {
        if (f.nid == nid) {
          printf("Function: %s, NID: %s\n", f.name.c_str(), f.nid.c_str());
          return true;
        }
      }

      for (const auto& f : lib.variables) {
        if (f.nid == nid) {
          printf("Variable: %s, NID: %s\n", f.name.c_str(), f.nid.c_str());
          return true;
        }
      }
    }
  }
  return false;
}

void viewPrxList(void) {
  for (const auto& x : prxTable) {
    printf("FileName: %s, ModuleName: %s\n", x.prx.c_str(), x.prx_name.c_str());
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }

  std::ifstream f("psplibdoc_660.xml", std::ios::in);
  std::stringstream input;
  if (!f)
    return 2;

  input << f.rdbuf();
  compile_prx_table(input.str());

  // Preprocessing for flags
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "--listprx") {
      viewPrxList();
      exit(0);
    }
  }

  bool status = false;
  for (int i = 1; i < argc; i++, status = false) {
    if (showPrxInfoByFile(argv[i]) == true)
      status = true;
    if (status)
      continue;
    if (findTypeByNID(argv[i]) == true)
      status = true;
    if (status)
      continue;
    if (findTypeByName(argv[i]) == true)
      status = true;
    if (status)
      continue;
    printf("Couldnt show info for %s\n", argv[i]);
  }
  return 0;
}
