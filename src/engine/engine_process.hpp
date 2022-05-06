#ifndef ENGINE_PROCESS_HPP
#define ENGINE_PROCESS_HPP

#include <boost/process.hpp>
#include "engine.hpp"

class ProcessEngine : public Engine {
   public:
    [[nodiscard]] ProcessEngine(const id_type id, const std::string &path)
        : Engine(id), m_child(path, boost::process::std_out > m_out, boost::process::std_in < m_in) {
    }

    [[nodiscard]] ProcessEngine(const id_type id, const std::string &path, callback_type recv, callback_type send)
        : Engine(id, recv, send), m_child(path, boost::process::std_out > m_out, boost::process::std_in < m_in) {
    }

    [[nodiscard]] virtual auto is_running() -> bool override {
        return m_child.running();
    }

   protected:
    auto send(const std::string &msg) -> void {
        m_send(msg);
        m_in << msg << std::endl;
    }

    auto wait_for(const std::string &msg) {
        std::string line;
        while (is_running()) {
            std::getline(m_out, line);
            m_recv(line);
            if (line == msg) {
                break;
            }
        }
    }

    auto wait_for(const std::string &msg, const callback_type func) {
        std::string line;
        while (is_running()) {
            std::getline(m_out, line);
            m_recv(line);
            if (line == msg) {
                break;
            } else {
                func(line);
            }
        }
    }

    auto wait_for(const std::function<bool(const std::string_view msg)> func) {
        std::string line;
        auto exit = false;
        while (is_running() && !exit) {
            std::getline(m_out, line);
            m_recv(line);
            exit = func(line);
        }
    }

   private:
    boost::process::opstream m_in;
    boost::process::ipstream m_out;
    boost::process::child m_child;
};

#endif
