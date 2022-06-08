/*
   Copyright (C) 2022
   Matthias P. Braendli, matthias.braendli@mpb.li

   http://opendigitalradio.org

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#pragma once

#include "edi/common.hpp"
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace EdiDecoder {

class EDIReceiver {
    public:
        using subchannel_handler = std::function<void(std::vector<uint8_t>&&, frame_timestamp_t, uint16_t /*dlfc*/)>;

        EDIReceiver();

        void set_verbose(bool verbose);

        /* Push a complete packet into the decoder. Useful for UDP and other
         * datagram-oriented protocols.
         */
        void push_packet(Packet &pack);

        /* Set the maximum delay in number of AF Packets before we
         * abandon decoding a given pseq.
         */
        void setMaxDelay(int num_af_packets);

        std::vector<uint8_t> received_tagpackets;

    private:
        void packet_completed();
        void tagpacket_handler(const std::vector<uint8_t>& tagpacket);

        std::string m_protocol = "";

        bool decode_starptr(const std::vector<uint8_t>& value, const tag_name_t& n);
        bool decode_stardmy(const std::vector<uint8_t>&, const tag_name_t&);

        TagDispatcher m_dispatcher;
};

}
