#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    class DebugUI : public Control {
        GDCLASS(DebugUI, Control);
        public:
            DebugUI();
            ~DebugUI();

            void _ready();
            void _process(double delta);

            void update_mesh_count(int count);

        private:
            void update_fps();
            void toggle_visibility();

            Label* m_fps_label;
            Label* m_mesh_count_label;
            int m_mesh_count;
            double m_fps_update_timer;

            bool m_visible;
        
        protected:
            static void _bind_methods();
    };
}