/*
   Copyright (C) 2022
   Matthias P. Braendli, matthias.braendli@mpb.li

    http://www.opendigitalradio.org
 */
/*
   This file is part of the ODR-mmbTools.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <chrono>
#include <iostream>
#include <iterator>
#include <memory>
#include <thread>
#include <vector>
#include <cmath>
#include <cstring>
#include "Socket.h"
#include "edi/ETIDecoder.hpp"


struct tagpacket_t {
    // source information
    std::string hostnames;

    uint16_t dlfc;
    std::vector<uint8_t> tagpacket;
    EdiDecoder::frame_timestamp_t timestamp;
    std::chrono::steady_clock::time_point received_at;
    EdiDecoder::seq_info_t seq;
};

struct source_t {
    source_t(std::string hostname, int port, bool enabled) : hostname(hostname), port(port), enabled(enabled) {}
    std::string hostname;
    int port;

    // User-controlled setting
    bool enabled;

    // Mode merging: active will be set for all enabled inputs.
    // Mode switching: only one input will be active
    bool active = false;

    bool connected = false;

    ssize_t num_connects = 0;
};

class Receiver : public EdiDecoder::ETIDataCollector {
    public:
        Receiver(source_t& source, std::function<void(tagpacket_t&& tagpacket, Receiver*)> push_tagpacket, bool verbose);
        Receiver(const Receiver&) = delete;
        Receiver operator=(const Receiver&) = delete;
        Receiver(Receiver&&) = default;
        Receiver& operator=(Receiver&&) = delete;

        // Tell the ETIWriter what EDI protocol we receive in *ptr.
        // This is not part of the ETI data, but is used as check
        virtual void update_protocol(
                const std::string& proto,
                uint16_t major,
                uint16_t minor) override { }

        // Update the data for the frame characterisation
        virtual void update_fc_data(const EdiDecoder::eti_fc_data& fc_data) override;

        // Ignore most events because we are interested in retransmitting EDI, not
        // decoding it
        virtual void update_fic(std::vector<uint8_t>&& fic) override { }
        virtual void update_err(uint8_t err) override { }
        virtual void update_edi_time(uint32_t utco, uint32_t seconds) override { }
        virtual void update_mnsc(uint16_t mnsc) override { }
        virtual void update_rfu(uint16_t rfu) override { }
        virtual void add_subchannel(EdiDecoder::eti_stc_data&& stc) override { }

        // Tell the ETIWriter that the AFPacket is complete
        virtual void assemble(EdiDecoder::ReceivedTagPacket&& tag_data) override;

        // Must return -1 if the socket is not poll()able
        int get_sockfd() const { return sock.get_sockfd(); }

        void receive();
        void tick();
        int get_margin_ms() const;

        std::chrono::system_clock::time_point get_systime_last_packet() const
        {
            return most_recent_rx_systime;
        }

        std::chrono::steady_clock::time_point get_time_last_packet() const
        {
            return most_recent_rx_time;
        }

        source_t& source;

        ssize_t num_late = 0;

    private:
        std::function<void(tagpacket_t&& tagpacket, Receiver*)> push_tagpacket_callback;
        EdiDecoder::ETIDecoder edi_decoder;
        uint16_t dlfc = 0;

        std::chrono::steady_clock::time_point reconnect_at = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point most_recent_rx_time = std::chrono::steady_clock::time_point();
        std::chrono::system_clock::time_point most_recent_rx_systime = std::chrono::system_clock::time_point();

        std::chrono::system_clock::duration margin = std::chrono::system_clock::duration::zero();


        Socket::TCPSocket sock;
};
