#include "register.hpp"
#include "gdextension_interface.h"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/godot.hpp"

#include "DebugUI.h"
#include "Planet.h"
#include "PlanetMesh.h"
#include "managers/CitiesManager.h"
#include "managers/ProvincesManager.h"

using namespace godot;

void initialize(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
  // GDREGISTER_CLASS(Planet);

  ClassDB::register_class<Planet>();
  ClassDB::register_class<PlanetMesh>();
  ClassDB::register_class<DebugUI>();
  ClassDB::register_class<ProvincesManager>();
  ClassDB::register_class<CitiesManager>();
}

void uninitialize(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
GDExtensionBool GDE_EXPORT
library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
             const GDExtensionClassLibraryPtr p_library,
             GDExtensionInitialization *p_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 p_initialization);
  init_obj.register_initializer(initialize);
  init_obj.register_terminator(uninitialize);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);
  return init_obj.init();
}
}
