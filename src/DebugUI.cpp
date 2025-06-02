#include "DebugUI.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <string>

namespace godot {

    DebugUI::DebugUI() : m_mesh_count(0), m_fps_update_timer(0) {
    }

    DebugUI::~DebugUI() {
    }

    void DebugUI::_ready() {
        // Create UI elements
        PanelContainer* panel = memnew(PanelContainer);
        add_child(panel);
        
        VBoxContainer* vbox = memnew(VBoxContainer);
        panel->add_child(vbox);
        
        // FPS display
        HBoxContainer* fps_hbox = memnew(HBoxContainer);
        vbox->add_child(fps_hbox);
        
        Label* fps_title = memnew(Label);
        fps_title->set_text("FPS: ");
        fps_hbox->add_child(fps_title);
        
        m_fps_label = memnew(Label);
        m_fps_label->set_text("0");
        fps_hbox->add_child(m_fps_label);
        
        // Mesh count display
        HBoxContainer* mesh_hbox = memnew(HBoxContainer);
        vbox->add_child(mesh_hbox);
        
        Label* mesh_title = memnew(Label);
        mesh_title->set_text("Meshes: ");
        mesh_hbox->add_child(mesh_title);
        
        m_mesh_count_label = memnew(Label);
        m_mesh_count_label->set_text("0");
        mesh_hbox->add_child(m_mesh_count_label);
        
        // Set panel properties
        panel->set_position(Vector2(10, 10));
        panel->set_size(Vector2(150, 80));
    }

    void DebugUI::_process(double delta) {
        // Update FPS counter every 0.5 seconds
        m_fps_update_timer += delta;
        if (m_fps_update_timer > 0.5) {
            update_fps();
            m_fps_update_timer = 0;
        }
    }

    void DebugUI::update_mesh_count(int count) {
        m_mesh_count = count;
        if (m_mesh_count_label) {
            m_mesh_count_label->set_text(std::to_string(count).c_str());
        }
    }

    void DebugUI::update_fps() {
        float fps = Engine::get_singleton()->get_frames_per_second();
        if (m_fps_label) {
            m_fps_label->set_text(std::to_string((int)fps).c_str());
        }
    }

    void DebugUI::_bind_methods() {

    }

}