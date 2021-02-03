#line 1 "ascii.rl"
/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 */

#include <nil/actor/core/ragel.hh>
#include "memcached.hh"
#include <memory>
#include <algorithm>
#include <functional>

using namespace nil::actor;

#line 95 "ascii.rl"

class memcache_ascii_parser : public ragel_parser_base<memcache_ascii_parser> {

#line 38 "achii.hh"
    static const int start = 1;
    static const int error = 0;

    static const int en_blob = 195;
    static const int en_main = 1;

#line 98 "ascii.rl"

public:
    enum class state {
        error,
        eof,
        cmd_set,
        cmd_cas,
        cmd_add,
        cmd_replace,
        cmd_get,
        cmd_gets,
        cmd_delete,
        cmd_flush_all,
        cmd_version,
        cmd_stats,
        cmd_stats_hash,
        cmd_incr,
        cmd_decr,
    };
    state _state;
    uint32_t _u32;
    uint64_t _u64;
    memcache::item_key _key;
    sstring _flags_str;
    uint32_t _expiration;
    uint32_t _size;
    sstring _size_str;
    uint32_t _size_left;
    uint64_t _version;
    sstring _blob;
    bool _noreply;
    std::vector<memcache::item_key> _keys;

public:
    void init() {
        init_base();
        _state = state::error;
        _keys.clear();

#line 85 "achii.hh"
        {
            _fsm_cs = (int)start;
            _fsm_top = 0;
        }

#line 135 "ascii.rl"
    }

    char *parse(char *p, char *pe, char *eof) {
        sstring_builder::guard g(_builder, p, pe);
        auto str = [this, &g, &p] {
            g.mark_end(p);
            return get_str();
        };
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmisleading-indentation"
#endif

#line 103 "achii.hh"
        {
            if (p == pe)
                goto _test_eof;
            goto _resume;

        _again : { }
            switch (_fsm_cs) {
                case 1:
                    goto _st1;
                case 0:
                    goto _st0;
                case 2:
                    goto _st2;
                case 3:
                    goto _st3;
                case 4:
                    goto _st4;
                case 5:
                    goto _st5;
                case 6:
                    goto _st6;
                case 7:
                    goto _st7;
                case 8:
                    goto _st8;
                case 9:
                    goto _st9;
                case 10:
                    goto _st10;
                case 11:
                    goto _st11;
                case 12:
                    goto _st12;
                case 13:
                    goto _st13;
                case 14:
                    goto _st14;
                case 15:
                    goto _st15;
                case 196:
                    goto _st196;
                case 16:
                    goto _st16;
                case 17:
                    goto _st17;
                case 18:
                    goto _st18;
                case 19:
                    goto _st19;
                case 20:
                    goto _st20;
                case 21:
                    goto _st21;
                case 22:
                    goto _st22;
                case 23:
                    goto _st23;
                case 24:
                    goto _st24;
                case 25:
                    goto _st25;
                case 26:
                    goto _st26;
                case 27:
                    goto _st27;
                case 28:
                    goto _st28;
                case 29:
                    goto _st29;
                case 30:
                    goto _st30;
                case 31:
                    goto _st31;
                case 32:
                    goto _st32;
                case 33:
                    goto _st33;
                case 34:
                    goto _st34;
                case 35:
                    goto _st35;
                case 36:
                    goto _st36;
                case 37:
                    goto _st37;
                case 38:
                    goto _st38;
                case 39:
                    goto _st39;
                case 40:
                    goto _st40;
                case 41:
                    goto _st41;
                case 42:
                    goto _st42;
                case 43:
                    goto _st43;
                case 44:
                    goto _st44;
                case 45:
                    goto _st45;
                case 46:
                    goto _st46;
                case 47:
                    goto _st47;
                case 48:
                    goto _st48;
                case 49:
                    goto _st49;
                case 50:
                    goto _st50;
                case 51:
                    goto _st51;
                case 52:
                    goto _st52;
                case 53:
                    goto _st53;
                case 54:
                    goto _st54;
                case 55:
                    goto _st55;
                case 56:
                    goto _st56;
                case 57:
                    goto _st57;
                case 58:
                    goto _st58;
                case 59:
                    goto _st59;
                case 60:
                    goto _st60;
                case 61:
                    goto _st61;
                case 62:
                    goto _st62;
                case 63:
                    goto _st63;
                case 64:
                    goto _st64;
                case 65:
                    goto _st65;
                case 66:
                    goto _st66;
                case 67:
                    goto _st67;
                case 68:
                    goto _st68;
                case 69:
                    goto _st69;
                case 70:
                    goto _st70;
                case 71:
                    goto _st71;
                case 197:
                    goto _st197;
                case 72:
                    goto _st72;
                case 73:
                    goto _st73;
                case 74:
                    goto _st74;
                case 75:
                    goto _st75;
                case 76:
                    goto _st76;
                case 77:
                    goto _st77;
                case 78:
                    goto _st78;
                case 79:
                    goto _st79;
                case 80:
                    goto _st80;
                case 81:
                    goto _st81;
                case 82:
                    goto _st82;
                case 83:
                    goto _st83;
                case 84:
                    goto _st84;
                case 85:
                    goto _st85;
                case 86:
                    goto _st86;
                case 87:
                    goto _st87;
                case 88:
                    goto _st88;
                case 89:
                    goto _st89;
                case 90:
                    goto _st90;
                case 91:
                    goto _st91;
                case 92:
                    goto _st92;
                case 93:
                    goto _st93;
                case 94:
                    goto _st94;
                case 95:
                    goto _st95;
                case 96:
                    goto _st96;
                case 97:
                    goto _st97;
                case 98:
                    goto _st98;
                case 99:
                    goto _st99;
                case 100:
                    goto _st100;
                case 101:
                    goto _st101;
                case 102:
                    goto _st102;
                case 103:
                    goto _st103;
                case 104:
                    goto _st104;
                case 105:
                    goto _st105;
                case 106:
                    goto _st106;
                case 198:
                    goto _st198;
                case 107:
                    goto _st107;
                case 108:
                    goto _st108;
                case 109:
                    goto _st109;
                case 110:
                    goto _st110;
                case 199:
                    goto _st199;
                case 111:
                    goto _st111;
                case 112:
                    goto _st112;
                case 113:
                    goto _st113;
                case 114:
                    goto _st114;
                case 115:
                    goto _st115;
                case 116:
                    goto _st116;
                case 117:
                    goto _st117;
                case 118:
                    goto _st118;
                case 119:
                    goto _st119;
                case 120:
                    goto _st120;
                case 121:
                    goto _st121;
                case 122:
                    goto _st122;
                case 123:
                    goto _st123;
                case 124:
                    goto _st124;
                case 125:
                    goto _st125;
                case 126:
                    goto _st126;
                case 127:
                    goto _st127;
                case 128:
                    goto _st128;
                case 129:
                    goto _st129;
                case 130:
                    goto _st130;
                case 131:
                    goto _st131;
                case 132:
                    goto _st132;
                case 133:
                    goto _st133;
                case 134:
                    goto _st134;
                case 135:
                    goto _st135;
                case 136:
                    goto _st136;
                case 137:
                    goto _st137;
                case 138:
                    goto _st138;
                case 139:
                    goto _st139;
                case 140:
                    goto _st140;
                case 141:
                    goto _st141;
                case 142:
                    goto _st142;
                case 143:
                    goto _st143;
                case 144:
                    goto _st144;
                case 145:
                    goto _st145;
                case 146:
                    goto _st146;
                case 147:
                    goto _st147;
                case 148:
                    goto _st148;
                case 149:
                    goto _st149;
                case 150:
                    goto _st150;
                case 151:
                    goto _st151;
                case 152:
                    goto _st152;
                case 153:
                    goto _st153;
                case 154:
                    goto _st154;
                case 155:
                    goto _st155;
                case 156:
                    goto _st156;
                case 157:
                    goto _st157;
                case 158:
                    goto _st158;
                case 159:
                    goto _st159;
                case 160:
                    goto _st160;
                case 161:
                    goto _st161;
                case 162:
                    goto _st162;
                case 163:
                    goto _st163;
                case 164:
                    goto _st164;
                case 165:
                    goto _st165;
                case 166:
                    goto _st166;
                case 167:
                    goto _st167;
                case 168:
                    goto _st168;
                case 169:
                    goto _st169;
                case 170:
                    goto _st170;
                case 171:
                    goto _st171;
                case 172:
                    goto _st172;
                case 173:
                    goto _st173;
                case 174:
                    goto _st174;
                case 175:
                    goto _st175;
                case 176:
                    goto _st176;
                case 177:
                    goto _st177;
                case 178:
                    goto _st178;
                case 179:
                    goto _st179;
                case 180:
                    goto _st180;
                case 181:
                    goto _st181;
                case 182:
                    goto _st182;
                case 183:
                    goto _st183;
                case 184:
                    goto _st184;
                case 185:
                    goto _st185;
                case 186:
                    goto _st186;
                case 187:
                    goto _st187;
                case 188:
                    goto _st188;
                case 189:
                    goto _st189;
                case 190:
                    goto _st190;
                case 191:
                    goto _st191;
                case 192:
                    goto _st192;
                case 193:
                    goto _st193;
                case 194:
                    goto _st194;
                case 195:
                    goto _st195;
                case 200:
                    goto _st200;
            }

        _resume : { }
            switch (_fsm_cs) {
                case 1:
                    goto st_case_1;
                case 0:
                    goto st_case_0;
                case 2:
                    goto st_case_2;
                case 3:
                    goto st_case_3;
                case 4:
                    goto st_case_4;
                case 5:
                    goto st_case_5;
                case 6:
                    goto st_case_6;
                case 7:
                    goto st_case_7;
                case 8:
                    goto st_case_8;
                case 9:
                    goto st_case_9;
                case 10:
                    goto st_case_10;
                case 11:
                    goto st_case_11;
                case 12:
                    goto st_case_12;
                case 13:
                    goto st_case_13;
                case 14:
                    goto st_case_14;
                case 15:
                    goto st_case_15;
                case 196:
                    goto st_case_196;
                case 16:
                    goto st_case_16;
                case 17:
                    goto st_case_17;
                case 18:
                    goto st_case_18;
                case 19:
                    goto st_case_19;
                case 20:
                    goto st_case_20;
                case 21:
                    goto st_case_21;
                case 22:
                    goto st_case_22;
                case 23:
                    goto st_case_23;
                case 24:
                    goto st_case_24;
                case 25:
                    goto st_case_25;
                case 26:
                    goto st_case_26;
                case 27:
                    goto st_case_27;
                case 28:
                    goto st_case_28;
                case 29:
                    goto st_case_29;
                case 30:
                    goto st_case_30;
                case 31:
                    goto st_case_31;
                case 32:
                    goto st_case_32;
                case 33:
                    goto st_case_33;
                case 34:
                    goto st_case_34;
                case 35:
                    goto st_case_35;
                case 36:
                    goto st_case_36;
                case 37:
                    goto st_case_37;
                case 38:
                    goto st_case_38;
                case 39:
                    goto st_case_39;
                case 40:
                    goto st_case_40;
                case 41:
                    goto st_case_41;
                case 42:
                    goto st_case_42;
                case 43:
                    goto st_case_43;
                case 44:
                    goto st_case_44;
                case 45:
                    goto st_case_45;
                case 46:
                    goto st_case_46;
                case 47:
                    goto st_case_47;
                case 48:
                    goto st_case_48;
                case 49:
                    goto st_case_49;
                case 50:
                    goto st_case_50;
                case 51:
                    goto st_case_51;
                case 52:
                    goto st_case_52;
                case 53:
                    goto st_case_53;
                case 54:
                    goto st_case_54;
                case 55:
                    goto st_case_55;
                case 56:
                    goto st_case_56;
                case 57:
                    goto st_case_57;
                case 58:
                    goto st_case_58;
                case 59:
                    goto st_case_59;
                case 60:
                    goto st_case_60;
                case 61:
                    goto st_case_61;
                case 62:
                    goto st_case_62;
                case 63:
                    goto st_case_63;
                case 64:
                    goto st_case_64;
                case 65:
                    goto st_case_65;
                case 66:
                    goto st_case_66;
                case 67:
                    goto st_case_67;
                case 68:
                    goto st_case_68;
                case 69:
                    goto st_case_69;
                case 70:
                    goto st_case_70;
                case 71:
                    goto st_case_71;
                case 197:
                    goto st_case_197;
                case 72:
                    goto st_case_72;
                case 73:
                    goto st_case_73;
                case 74:
                    goto st_case_74;
                case 75:
                    goto st_case_75;
                case 76:
                    goto st_case_76;
                case 77:
                    goto st_case_77;
                case 78:
                    goto st_case_78;
                case 79:
                    goto st_case_79;
                case 80:
                    goto st_case_80;
                case 81:
                    goto st_case_81;
                case 82:
                    goto st_case_82;
                case 83:
                    goto st_case_83;
                case 84:
                    goto st_case_84;
                case 85:
                    goto st_case_85;
                case 86:
                    goto st_case_86;
                case 87:
                    goto st_case_87;
                case 88:
                    goto st_case_88;
                case 89:
                    goto st_case_89;
                case 90:
                    goto st_case_90;
                case 91:
                    goto st_case_91;
                case 92:
                    goto st_case_92;
                case 93:
                    goto st_case_93;
                case 94:
                    goto st_case_94;
                case 95:
                    goto st_case_95;
                case 96:
                    goto st_case_96;
                case 97:
                    goto st_case_97;
                case 98:
                    goto st_case_98;
                case 99:
                    goto st_case_99;
                case 100:
                    goto st_case_100;
                case 101:
                    goto st_case_101;
                case 102:
                    goto st_case_102;
                case 103:
                    goto st_case_103;
                case 104:
                    goto st_case_104;
                case 105:
                    goto st_case_105;
                case 106:
                    goto st_case_106;
                case 198:
                    goto st_case_198;
                case 107:
                    goto st_case_107;
                case 108:
                    goto st_case_108;
                case 109:
                    goto st_case_109;
                case 110:
                    goto st_case_110;
                case 199:
                    goto st_case_199;
                case 111:
                    goto st_case_111;
                case 112:
                    goto st_case_112;
                case 113:
                    goto st_case_113;
                case 114:
                    goto st_case_114;
                case 115:
                    goto st_case_115;
                case 116:
                    goto st_case_116;
                case 117:
                    goto st_case_117;
                case 118:
                    goto st_case_118;
                case 119:
                    goto st_case_119;
                case 120:
                    goto st_case_120;
                case 121:
                    goto st_case_121;
                case 122:
                    goto st_case_122;
                case 123:
                    goto st_case_123;
                case 124:
                    goto st_case_124;
                case 125:
                    goto st_case_125;
                case 126:
                    goto st_case_126;
                case 127:
                    goto st_case_127;
                case 128:
                    goto st_case_128;
                case 129:
                    goto st_case_129;
                case 130:
                    goto st_case_130;
                case 131:
                    goto st_case_131;
                case 132:
                    goto st_case_132;
                case 133:
                    goto st_case_133;
                case 134:
                    goto st_case_134;
                case 135:
                    goto st_case_135;
                case 136:
                    goto st_case_136;
                case 137:
                    goto st_case_137;
                case 138:
                    goto st_case_138;
                case 139:
                    goto st_case_139;
                case 140:
                    goto st_case_140;
                case 141:
                    goto st_case_141;
                case 142:
                    goto st_case_142;
                case 143:
                    goto st_case_143;
                case 144:
                    goto st_case_144;
                case 145:
                    goto st_case_145;
                case 146:
                    goto st_case_146;
                case 147:
                    goto st_case_147;
                case 148:
                    goto st_case_148;
                case 149:
                    goto st_case_149;
                case 150:
                    goto st_case_150;
                case 151:
                    goto st_case_151;
                case 152:
                    goto st_case_152;
                case 153:
                    goto st_case_153;
                case 154:
                    goto st_case_154;
                case 155:
                    goto st_case_155;
                case 156:
                    goto st_case_156;
                case 157:
                    goto st_case_157;
                case 158:
                    goto st_case_158;
                case 159:
                    goto st_case_159;
                case 160:
                    goto st_case_160;
                case 161:
                    goto st_case_161;
                case 162:
                    goto st_case_162;
                case 163:
                    goto st_case_163;
                case 164:
                    goto st_case_164;
                case 165:
                    goto st_case_165;
                case 166:
                    goto st_case_166;
                case 167:
                    goto st_case_167;
                case 168:
                    goto st_case_168;
                case 169:
                    goto st_case_169;
                case 170:
                    goto st_case_170;
                case 171:
                    goto st_case_171;
                case 172:
                    goto st_case_172;
                case 173:
                    goto st_case_173;
                case 174:
                    goto st_case_174;
                case 175:
                    goto st_case_175;
                case 176:
                    goto st_case_176;
                case 177:
                    goto st_case_177;
                case 178:
                    goto st_case_178;
                case 179:
                    goto st_case_179;
                case 180:
                    goto st_case_180;
                case 181:
                    goto st_case_181;
                case 182:
                    goto st_case_182;
                case 183:
                    goto st_case_183;
                case 184:
                    goto st_case_184;
                case 185:
                    goto st_case_185;
                case 186:
                    goto st_case_186;
                case 187:
                    goto st_case_187;
                case 188:
                    goto st_case_188;
                case 189:
                    goto st_case_189;
                case 190:
                    goto st_case_190;
                case 191:
                    goto st_case_191;
                case 192:
                    goto st_case_192;
                case 193:
                    goto st_case_193;
                case 194:
                    goto st_case_194;
                case 195:
                    goto st_case_195;
                case 200:
                    goto st_case_200;
            }
            goto st_out;
        _ctr1 : {
#line 85 "ascii.rl"
            _state = state::eof;
        }

#line 725 "achii.hh"

            goto _st1;
        _st1:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof1;
        st_case_1:
            switch (((*(p)))) {
                case 97: {
                    goto _st2;
                }
                case 99: {
                    goto _st24;
                }
                case 100: {
                    goto _st48;
                }
                case 102: {
                    goto _st81;
                }
                case 103: {
                    goto _st101;
                }
                case 105: {
                    goto _st111;
                }
                case 114: {
                    goto _st128;
                }
                case 115: {
                    goto _st154;
                }
                case 118: {
                    goto _st187;
                }
            }
            { goto _st0; }
        st_case_0:
        _st0:
            _fsm_cs = 0;
            goto _pop;
        _st2:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof2;
        st_case_2:
            if (((*(p))) == 100) {
                goto _st3;
            }
            { goto _st0; }
        _st3:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof3;
        st_case_3:
            if (((*(p))) == 100) {
                goto _st4;
            }
            { goto _st0; }
        _st4:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof4;
        st_case_4:
            if (((*(p))) == 32) {
                goto _st5;
            }
            { goto _st0; }
        _st5:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof5;
        st_case_5:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr14; }
        _ctr14 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 850 "achii.hh"

            goto _st6;
        _st6:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof6;
        st_case_6:
            if (((*(p))) == 32) {
                goto _ctr16;
            }
            { goto _st6; }
        _ctr16 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 875 "achii.hh"

            goto _st7;
        _st7:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof7;
        st_case_7:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr18;
            }
            { goto _st0; }
        _ctr18 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 902 "achii.hh"

            goto _st8;
        _st8:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof8;
        st_case_8:
            if (((*(p))) == 32) {
                goto _ctr20;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _st8;
            }
            { goto _st0; }
        _ctr20 : {
#line 62 "ascii.rl"
            _flags_str = str();
        }

#line 930 "achii.hh"

            goto _st9;
        _st9:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof9;
        st_case_9:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr22;
            }
            { goto _st0; }
        _ctr22 : {
#line 59 "ascii.rl"
            _u32 = 0;
        }

#line 955 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 961 "achii.hh"

            goto _st10;
        _ctr25 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 969 "achii.hh"

            goto _st10;
        _st10:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof10;
        st_case_10:
            if (((*(p))) == 32) {
                goto _ctr24;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr25;
            }
            { goto _st0; }
        _ctr24 : {
#line 63 "ascii.rl"
            _expiration = _u32;
        }

#line 997 "achii.hh"

            goto _st11;
        _st11:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof11;
        st_case_11:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr27;
            }
            { goto _st0; }
        _ctr31 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 1022 "achii.hh"

            goto _st12;
        _ctr27 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 1032 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 = 0;
            }

#line 1038 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 1044 "achii.hh"

            goto _st12;
        _st12:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof12;
        st_case_12:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr29;
                }
                case 32: {
                    goto _ctr30;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr31;
            }
            { goto _st0; }
        _ctr29 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 1077 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 1083 "achii.hh"

            goto _st13;
        _st13:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof13;
        st_case_13:
            if (((*(p))) == 10) {
                goto _ctr33;
            }
            { goto _st0; }
        _ctr33 : {
#line 70 "ascii.rl"
            {
                {
#line 87 "ascii.rl"

                    prepush();
                }
                _fsm_stack[_fsm_top] = 14;
                _fsm_top += 1;
                goto _st195;
            }
        }

#line 1113 "achii.hh"

            goto _st14;
        _st14:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof14;
        st_case_14:
            if (((*(p))) == 13) {
                goto _st15;
            }
            { goto _st0; }
        _st15:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof15;
        st_case_15:
            if (((*(p))) == 10) {
                goto _ctr36;
            }
            { goto _st0; }
        _ctr36 : {
#line 72 "ascii.rl"
            _state = state::cmd_add;
        }

#line 1155 "achii.hh"

            goto _st196;
        _ctr76 : {
#line 74 "ascii.rl"
            _state = state::cmd_cas;
        }

#line 1163 "achii.hh"

            goto _st196;
        _ctr101 : {
#line 83 "ascii.rl"
            _state = state::cmd_decr;
        }

#line 1171 "achii.hh"

            goto _st196;
        _ctr131 : {
#line 77 "ascii.rl"
            _state = state::cmd_delete;
        }

#line 1179 "achii.hh"

            goto _st196;
        _ctr143 : {
#line 78 "ascii.rl"
            _state = state::cmd_flush_all;
        }

#line 1187 "achii.hh"

            goto _st196;
        _ctr190 : {
#line 82 "ascii.rl"
            _state = state::cmd_incr;
        }

#line 1195 "achii.hh"

            goto _st196;
        _ctr229 : {
#line 73 "ascii.rl"
            _state = state::cmd_replace;
        }

#line 1203 "achii.hh"

            goto _st196;
        _ctr265 : {
#line 71 "ascii.rl"
            _state = state::cmd_set;
        }

#line 1211 "achii.hh"

            goto _st196;
        _ctr280 : {
#line 80 "ascii.rl"
            _state = state::cmd_stats;
        }

#line 1219 "achii.hh"

            goto _st196;
        _ctr286 : {
#line 81 "ascii.rl"
            _state = state::cmd_stats_hash;
        }

#line 1227 "achii.hh"

            goto _st196;
        _ctr294 : {
#line 79 "ascii.rl"
            _state = state::cmd_version;
        }

#line 1235 "achii.hh"

            goto _st196;
        _st196:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof196;
        st_case_196 : { goto _st0; }
        _ctr30 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 1257 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 1263 "achii.hh"

            goto _st16;
        _st16:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof16;
        st_case_16:
            if (((*(p))) == 110) {
                goto _st17;
            }
            { goto _st0; }
        _st17:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof17;
        st_case_17:
            if (((*(p))) == 111) {
                goto _st18;
            }
            { goto _st0; }
        _st18:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof18;
        st_case_18:
            if (((*(p))) == 114) {
                goto _st19;
            }
            { goto _st0; }
        _st19:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof19;
        st_case_19:
            if (((*(p))) == 101) {
                goto _st20;
            }
            { goto _st0; }
        _st20:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof20;
        st_case_20:
            if (((*(p))) == 112) {
                goto _st21;
            }
            { goto _st0; }
        _st21:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof21;
        st_case_21:
            if (((*(p))) == 108) {
                goto _st22;
            }
            { goto _st0; }
        _st22:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof22;
        st_case_22:
            if (((*(p))) == 121) {
                goto _ctr44;
            }
            { goto _st0; }
        _ctr44 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 1390 "achii.hh"

            goto _st23;
        _st23:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof23;
        st_case_23:
            if (((*(p))) == 13) {
                goto _st13;
            }
            { goto _st0; }
        _st24:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof24;
        st_case_24:
            if (((*(p))) == 97) {
                goto _st25;
            }
            { goto _st0; }
        _st25:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof25;
        st_case_25:
            if (((*(p))) == 115) {
                goto _st26;
            }
            { goto _st0; }
        _st26:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof26;
        st_case_26:
            if (((*(p))) == 32) {
                goto _st27;
            }
            { goto _st0; }
        _st27:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof27;
        st_case_27:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr49; }
        _ctr49 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 1485 "achii.hh"

            goto _st28;
        _st28:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof28;
        st_case_28:
            if (((*(p))) == 32) {
                goto _ctr51;
            }
            { goto _st28; }
        _ctr51 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 1510 "achii.hh"

            goto _st29;
        _st29:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof29;
        st_case_29:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr53;
            }
            { goto _st0; }
        _ctr53 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 1537 "achii.hh"

            goto _st30;
        _st30:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof30;
        st_case_30:
            if (((*(p))) == 32) {
                goto _ctr55;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _st30;
            }
            { goto _st0; }
        _ctr55 : {
#line 62 "ascii.rl"
            _flags_str = str();
        }

#line 1565 "achii.hh"

            goto _st31;
        _st31:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof31;
        st_case_31:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr57;
            }
            { goto _st0; }
        _ctr57 : {
#line 59 "ascii.rl"
            _u32 = 0;
        }

#line 1590 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 1596 "achii.hh"

            goto _st32;
        _ctr60 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 1604 "achii.hh"

            goto _st32;
        _st32:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof32;
        st_case_32:
            if (((*(p))) == 32) {
                goto _ctr59;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr60;
            }
            { goto _st0; }
        _ctr59 : {
#line 63 "ascii.rl"
            _expiration = _u32;
        }

#line 1632 "achii.hh"

            goto _st33;
        _st33:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof33;
        st_case_33:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr62;
            }
            { goto _st0; }
        _ctr65 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 1657 "achii.hh"

            goto _st34;
        _ctr62 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 1667 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 = 0;
            }

#line 1673 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 1679 "achii.hh"

            goto _st34;
        _st34:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof34;
        st_case_34:
            if (((*(p))) == 32) {
                goto _ctr64;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr65;
            }
            { goto _st0; }
        _ctr64 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 1707 "achii.hh"

            goto _st35;
        _st35:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof35;
        st_case_35:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr67;
            }
            { goto _st0; }
        _ctr67 : {
#line 60 "ascii.rl"
            _u64 = 0;
        }

#line 1732 "achii.hh"

            {
#line 60 "ascii.rl"
                _u64 *= 10;
                _u64 += (((*(p)))) - '0';
            }

#line 1738 "achii.hh"

            goto _st36;
        _ctr71 : {
#line 60 "ascii.rl"
            _u64 *= 10;
            _u64 += (((*(p)))) - '0';
        }

#line 1746 "achii.hh"

            goto _st36;
        _st36:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof36;
        st_case_36:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr69;
                }
                case 32: {
                    goto _ctr70;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr71;
            }
            { goto _st0; }
        _ctr69 : {
#line 68 "ascii.rl"
            _version = _u64;
        }

#line 1779 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 1785 "achii.hh"

            goto _st37;
        _st37:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof37;
        st_case_37:
            if (((*(p))) == 10) {
                goto _ctr73;
            }
            { goto _st0; }
        _ctr73 : {
#line 74 "ascii.rl"
            {
                {
#line 87 "ascii.rl"

                    prepush();
                }
                _fsm_stack[_fsm_top] = 38;
                _fsm_top += 1;
                goto _st195;
            }
        }

#line 1815 "achii.hh"

            goto _st38;
        _st38:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof38;
        st_case_38:
            if (((*(p))) == 13) {
                goto _st39;
            }
            { goto _st0; }
        _st39:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof39;
        st_case_39:
            if (((*(p))) == 10) {
                goto _ctr76;
            }
            { goto _st0; }
        _ctr70 : {
#line 68 "ascii.rl"
            _version = _u64;
        }

#line 1857 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 1863 "achii.hh"

            goto _st40;
        _st40:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof40;
        st_case_40:
            if (((*(p))) == 110) {
                goto _st41;
            }
            { goto _st0; }
        _st41:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof41;
        st_case_41:
            if (((*(p))) == 111) {
                goto _st42;
            }
            { goto _st0; }
        _st42:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof42;
        st_case_42:
            if (((*(p))) == 114) {
                goto _st43;
            }
            { goto _st0; }
        _st43:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof43;
        st_case_43:
            if (((*(p))) == 101) {
                goto _st44;
            }
            { goto _st0; }
        _st44:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof44;
        st_case_44:
            if (((*(p))) == 112) {
                goto _st45;
            }
            { goto _st0; }
        _st45:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof45;
        st_case_45:
            if (((*(p))) == 108) {
                goto _st46;
            }
            { goto _st0; }
        _st46:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof46;
        st_case_46:
            if (((*(p))) == 121) {
                goto _ctr84;
            }
            { goto _st0; }
        _ctr84 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 1990 "achii.hh"

            goto _st47;
        _st47:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof47;
        st_case_47:
            if (((*(p))) == 13) {
                goto _st37;
            }
            { goto _st0; }
        _st48:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof48;
        st_case_48:
            if (((*(p))) == 101) {
                goto _st49;
            }
            { goto _st0; }
        _st49:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof49;
        st_case_49:
            switch (((*(p)))) {
                case 99: {
                    goto _st50;
                }
                case 108: {
                    goto _st65;
                }
            }
            { goto _st0; }
        _st50:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof50;
        st_case_50:
            if (((*(p))) == 114) {
                goto _st51;
            }
            { goto _st0; }
        _st51:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof51;
        st_case_51:
            if (((*(p))) == 32) {
                goto _st52;
            }
            { goto _st0; }
        _st52:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof52;
        st_case_52:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr91; }
        _ctr91 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 2107 "achii.hh"

            goto _st53;
        _st53:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof53;
        st_case_53:
            if (((*(p))) == 32) {
                goto _ctr93;
            }
            { goto _st53; }
        _ctr93 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 2132 "achii.hh"

            goto _st54;
        _st54:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof54;
        st_case_54:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr95;
            }
            { goto _st0; }
        _ctr95 : {
#line 60 "ascii.rl"
            _u64 = 0;
        }

#line 2157 "achii.hh"

            {
#line 60 "ascii.rl"
                _u64 *= 10;
                _u64 += (((*(p)))) - '0';
            }

#line 2163 "achii.hh"

            goto _st55;
        _ctr99 : {
#line 60 "ascii.rl"
            _u64 *= 10;
            _u64 += (((*(p)))) - '0';
        }

#line 2171 "achii.hh"

            goto _st55;
        _st55:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof55;
        st_case_55:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr97;
                }
                case 32: {
                    goto _ctr98;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr99;
            }
            { goto _st0; }
        _ctr97 : {
#line 66 "ascii.rl"
            _noreply = false;
        }

#line 2204 "achii.hh"

            goto _st56;
        _st56:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof56;
        st_case_56:
            if (((*(p))) == 10) {
                goto _ctr101;
            }
            { goto _st0; }
        _ctr98 : {
#line 66 "ascii.rl"
            _noreply = false;
        }

#line 2229 "achii.hh"

            goto _st57;
        _st57:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof57;
        st_case_57:
            if (((*(p))) == 110) {
                goto _st58;
            }
            { goto _st0; }
        _st58:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof58;
        st_case_58:
            if (((*(p))) == 111) {
                goto _st59;
            }
            { goto _st0; }
        _st59:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof59;
        st_case_59:
            if (((*(p))) == 114) {
                goto _st60;
            }
            { goto _st0; }
        _st60:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof60;
        st_case_60:
            if (((*(p))) == 101) {
                goto _st61;
            }
            { goto _st0; }
        _st61:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof61;
        st_case_61:
            if (((*(p))) == 112) {
                goto _st62;
            }
            { goto _st0; }
        _st62:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof62;
        st_case_62:
            if (((*(p))) == 108) {
                goto _st63;
            }
            { goto _st0; }
        _st63:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof63;
        st_case_63:
            if (((*(p))) == 121) {
                goto _ctr109;
            }
            { goto _st0; }
        _ctr109 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 2356 "achii.hh"

            goto _st64;
        _st64:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof64;
        st_case_64:
            if (((*(p))) == 13) {
                goto _st56;
            }
            { goto _st0; }
        _st65:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof65;
        st_case_65:
            if (((*(p))) == 101) {
                goto _st66;
            }
            { goto _st0; }
        _st66:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof66;
        st_case_66:
            if (((*(p))) == 116) {
                goto _st67;
            }
            { goto _st0; }
        _st67:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof67;
        st_case_67:
            if (((*(p))) == 101) {
                goto _st68;
            }
            { goto _st0; }
        _st68:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof68;
        st_case_68:
            if (((*(p))) == 32) {
                goto _st69;
            }
            { goto _st0; }
        _st69:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof69;
        st_case_69:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr115; }
        _ctr115 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 2468 "achii.hh"

            goto _st70;
        _st70:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof70;
        st_case_70:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr117;
                }
                case 32: {
                    goto _ctr118;
                }
            }
            { goto _st70; }
        _ctr117 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 2498 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 2504 "achii.hh"

            goto _st71;
        _st71:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof71;
        st_case_71:
            switch (((*(p)))) {
                case 10: {
                    goto _ctr120;
                }
                case 13: {
                    goto _ctr117;
                }
                case 32: {
                    goto _ctr118;
                }
            }
            { goto _st70; }
        _ctr120 : {
#line 77 "ascii.rl"
            _state = state::cmd_delete;
        }

#line 2537 "achii.hh"

            goto _st197;
        _st197:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof197;
        st_case_197:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr117;
                }
                case 32: {
                    goto _ctr118;
                }
            }
            { goto _st70; }
        _ctr118 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 2567 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 2573 "achii.hh"

            goto _st72;
        _st72:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof72;
        st_case_72:
            if (((*(p))) == 110) {
                goto _st73;
            }
            { goto _st0; }
        _st73:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof73;
        st_case_73:
            if (((*(p))) == 111) {
                goto _st74;
            }
            { goto _st0; }
        _st74:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof74;
        st_case_74:
            if (((*(p))) == 114) {
                goto _st75;
            }
            { goto _st0; }
        _st75:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof75;
        st_case_75:
            if (((*(p))) == 101) {
                goto _st76;
            }
            { goto _st0; }
        _st76:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof76;
        st_case_76:
            if (((*(p))) == 112) {
                goto _st77;
            }
            { goto _st0; }
        _st77:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof77;
        st_case_77:
            if (((*(p))) == 108) {
                goto _st78;
            }
            { goto _st0; }
        _st78:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof78;
        st_case_78:
            if (((*(p))) == 121) {
                goto _ctr128;
            }
            { goto _st0; }
        _ctr128 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 2700 "achii.hh"

            goto _st79;
        _st79:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof79;
        st_case_79:
            if (((*(p))) == 13) {
                goto _st80;
            }
            { goto _st0; }
        _st80:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof80;
        st_case_80:
            if (((*(p))) == 10) {
                goto _ctr131;
            }
            { goto _st0; }
        _st81:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof81;
        st_case_81:
            if (((*(p))) == 108) {
                goto _st82;
            }
            { goto _st0; }
        _st82:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof82;
        st_case_82:
            if (((*(p))) == 117) {
                goto _st83;
            }
            { goto _st0; }
        _st83:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof83;
        st_case_83:
            if (((*(p))) == 115) {
                goto _st84;
            }
            { goto _st0; }
        _st84:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof84;
        st_case_84:
            if (((*(p))) == 104) {
                goto _st85;
            }
            { goto _st0; }
        _st85:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof85;
        st_case_85:
            if (((*(p))) == 95) {
                goto _st86;
            }
            { goto _st0; }
        _st86:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof86;
        st_case_86:
            if (((*(p))) == 97) {
                goto _st87;
            }
            { goto _st0; }
        _st87:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof87;
        st_case_87:
            if (((*(p))) == 108) {
                goto _st88;
            }
            { goto _st0; }
        _st88:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof88;
        st_case_88:
            if (((*(p))) == 108) {
                goto _st89;
            }
            { goto _st0; }
        _st89:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof89;
        st_case_89:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr140;
                }
                case 32: {
                    goto _ctr141;
                }
            }
            { goto _st0; }
        _ctr140 : {
#line 67 "ascii.rl"
            _expiration = 0;
        }

#line 2900 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 2906 "achii.hh"

            goto _st90;
        _ctr148 : {
#line 63 "ascii.rl"
            _expiration = _u32;
        }

#line 2914 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 2920 "achii.hh"

            goto _st90;
        _st90:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof90;
        st_case_90:
            if (((*(p))) == 10) {
                goto _ctr143;
            }
            { goto _st0; }
        _ctr141 : {
#line 67 "ascii.rl"
            _expiration = 0;
        }

#line 2945 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 2951 "achii.hh"

            goto _st91;
        _st91:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof91;
        st_case_91:
            if (((*(p))) == 110) {
                goto _st94;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr145;
            }
            { goto _st0; }
        _ctr145 : {
#line 59 "ascii.rl"
            _u32 = 0;
        }

#line 2979 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 2985 "achii.hh"

            goto _st92;
        _ctr150 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 2993 "achii.hh"

            goto _st92;
        _st92:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof92;
        st_case_92:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr148;
                }
                case 32: {
                    goto _ctr149;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr150;
            }
            { goto _st0; }
        _ctr149 : {
#line 63 "ascii.rl"
            _expiration = _u32;
        }

#line 3026 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 3032 "achii.hh"

            goto _st93;
        _st93:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof93;
        st_case_93:
            if (((*(p))) == 110) {
                goto _st94;
            }
            { goto _st0; }
        _st94:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof94;
        st_case_94:
            if (((*(p))) == 111) {
                goto _st95;
            }
            { goto _st0; }
        _st95:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof95;
        st_case_95:
            if (((*(p))) == 114) {
                goto _st96;
            }
            { goto _st0; }
        _st96:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof96;
        st_case_96:
            if (((*(p))) == 101) {
                goto _st97;
            }
            { goto _st0; }
        _st97:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof97;
        st_case_97:
            if (((*(p))) == 112) {
                goto _st98;
            }
            { goto _st0; }
        _st98:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof98;
        st_case_98:
            if (((*(p))) == 108) {
                goto _st99;
            }
            { goto _st0; }
        _st99:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof99;
        st_case_99:
            if (((*(p))) == 121) {
                goto _ctr157;
            }
            { goto _st0; }
        _ctr157 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 3159 "achii.hh"

            goto _st100;
        _st100:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof100;
        st_case_100:
            if (((*(p))) == 13) {
                goto _st90;
            }
            { goto _st0; }
        _st101:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof101;
        st_case_101:
            if (((*(p))) == 101) {
                goto _st102;
            }
            { goto _st0; }
        _st102:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof102;
        st_case_102:
            if (((*(p))) == 116) {
                goto _st103;
            }
            { goto _st0; }
        _st103:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof103;
        st_case_103:
            switch (((*(p)))) {
                case 32: {
                    goto _st104;
                }
                case 115: {
                    goto _st107;
                }
            }
            { goto _st0; }
        _ctr166 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 3240 "achii.hh"

            {
#line 75 "ascii.rl"
                _keys.emplace_back(std::move(_key));
            }

#line 3246 "achii.hh"

            goto _st104;
        _st104:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof104;
        st_case_104:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr163; }
        _ctr163 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 3273 "achii.hh"

            goto _st105;
        _st105:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof105;
        st_case_105:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr165;
                }
                case 32: {
                    goto _ctr166;
                }
            }
            { goto _st105; }
        _ctr165 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 3303 "achii.hh"

            {
#line 75 "ascii.rl"
                _keys.emplace_back(std::move(_key));
            }

#line 3309 "achii.hh"

            goto _st106;
        _st106:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof106;
        st_case_106:
            switch (((*(p)))) {
                case 10: {
                    goto _ctr168;
                }
                case 13: {
                    goto _ctr165;
                }
                case 32: {
                    goto _ctr166;
                }
            }
            { goto _st105; }
        _ctr168 : {
#line 75 "ascii.rl"
            _state = state::cmd_get;
        }

#line 3342 "achii.hh"

            goto _st198;
        _st198:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof198;
        st_case_198:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr165;
                }
                case 32: {
                    goto _ctr166;
                }
            }
            { goto _st105; }
        _st107:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof107;
        st_case_107:
            if (((*(p))) == 32) {
                goto _st108;
            }
            { goto _st0; }
        _ctr173 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 3389 "achii.hh"

            {
#line 76 "ascii.rl"
                _keys.emplace_back(std::move(_key));
            }

#line 3395 "achii.hh"

            goto _st108;
        _st108:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof108;
        st_case_108:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr170; }
        _ctr170 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 3422 "achii.hh"

            goto _st109;
        _st109:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof109;
        st_case_109:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr172;
                }
                case 32: {
                    goto _ctr173;
                }
            }
            { goto _st109; }
        _ctr172 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 3452 "achii.hh"

            {
#line 76 "ascii.rl"
                _keys.emplace_back(std::move(_key));
            }

#line 3458 "achii.hh"

            goto _st110;
        _st110:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof110;
        st_case_110:
            switch (((*(p)))) {
                case 10: {
                    goto _ctr175;
                }
                case 13: {
                    goto _ctr172;
                }
                case 32: {
                    goto _ctr173;
                }
            }
            { goto _st109; }
        _ctr175 : {
#line 76 "ascii.rl"
            _state = state::cmd_gets;
        }

#line 3491 "achii.hh"

            goto _st199;
        _st199:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof199;
        st_case_199:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr172;
                }
                case 32: {
                    goto _ctr173;
                }
            }
            { goto _st109; }
        _st111:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof111;
        st_case_111:
            if (((*(p))) == 110) {
                goto _st112;
            }
            { goto _st0; }
        _st112:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof112;
        st_case_112:
            if (((*(p))) == 99) {
                goto _st113;
            }
            { goto _st0; }
        _st113:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof113;
        st_case_113:
            if (((*(p))) == 114) {
                goto _st114;
            }
            { goto _st0; }
        _st114:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof114;
        st_case_114:
            if (((*(p))) == 32) {
                goto _st115;
            }
            { goto _st0; }
        _st115:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof115;
        st_case_115:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr180; }
        _ctr180 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 3608 "achii.hh"

            goto _st116;
        _st116:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof116;
        st_case_116:
            if (((*(p))) == 32) {
                goto _ctr182;
            }
            { goto _st116; }
        _ctr182 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 3633 "achii.hh"

            goto _st117;
        _st117:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof117;
        st_case_117:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr184;
            }
            { goto _st0; }
        _ctr184 : {
#line 60 "ascii.rl"
            _u64 = 0;
        }

#line 3658 "achii.hh"

            {
#line 60 "ascii.rl"
                _u64 *= 10;
                _u64 += (((*(p)))) - '0';
            }

#line 3664 "achii.hh"

            goto _st118;
        _ctr188 : {
#line 60 "ascii.rl"
            _u64 *= 10;
            _u64 += (((*(p)))) - '0';
        }

#line 3672 "achii.hh"

            goto _st118;
        _st118:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof118;
        st_case_118:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr186;
                }
                case 32: {
                    goto _ctr187;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr188;
            }
            { goto _st0; }
        _ctr186 : {
#line 66 "ascii.rl"
            _noreply = false;
        }

#line 3705 "achii.hh"

            goto _st119;
        _st119:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof119;
        st_case_119:
            if (((*(p))) == 10) {
                goto _ctr190;
            }
            { goto _st0; }
        _ctr187 : {
#line 66 "ascii.rl"
            _noreply = false;
        }

#line 3730 "achii.hh"

            goto _st120;
        _st120:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof120;
        st_case_120:
            if (((*(p))) == 110) {
                goto _st121;
            }
            { goto _st0; }
        _st121:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof121;
        st_case_121:
            if (((*(p))) == 111) {
                goto _st122;
            }
            { goto _st0; }
        _st122:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof122;
        st_case_122:
            if (((*(p))) == 114) {
                goto _st123;
            }
            { goto _st0; }
        _st123:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof123;
        st_case_123:
            if (((*(p))) == 101) {
                goto _st124;
            }
            { goto _st0; }
        _st124:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof124;
        st_case_124:
            if (((*(p))) == 112) {
                goto _st125;
            }
            { goto _st0; }
        _st125:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof125;
        st_case_125:
            if (((*(p))) == 108) {
                goto _st126;
            }
            { goto _st0; }
        _st126:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof126;
        st_case_126:
            if (((*(p))) == 121) {
                goto _ctr198;
            }
            { goto _st0; }
        _ctr198 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 3857 "achii.hh"

            goto _st127;
        _st127:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof127;
        st_case_127:
            if (((*(p))) == 13) {
                goto _st119;
            }
            { goto _st0; }
        _st128:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof128;
        st_case_128:
            if (((*(p))) == 101) {
                goto _st129;
            }
            { goto _st0; }
        _st129:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof129;
        st_case_129:
            if (((*(p))) == 112) {
                goto _st130;
            }
            { goto _st0; }
        _st130:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof130;
        st_case_130:
            if (((*(p))) == 108) {
                goto _st131;
            }
            { goto _st0; }
        _st131:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof131;
        st_case_131:
            if (((*(p))) == 97) {
                goto _st132;
            }
            { goto _st0; }
        _st132:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof132;
        st_case_132:
            if (((*(p))) == 99) {
                goto _st133;
            }
            { goto _st0; }
        _st133:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof133;
        st_case_133:
            if (((*(p))) == 101) {
                goto _st134;
            }
            { goto _st0; }
        _st134:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof134;
        st_case_134:
            if (((*(p))) == 32) {
                goto _st135;
            }
            { goto _st0; }
        _st135:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof135;
        st_case_135:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr207; }
        _ctr207 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 4020 "achii.hh"

            goto _st136;
        _st136:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof136;
        st_case_136:
            if (((*(p))) == 32) {
                goto _ctr209;
            }
            { goto _st136; }
        _ctr209 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 4045 "achii.hh"

            goto _st137;
        _st137:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof137;
        st_case_137:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr211;
            }
            { goto _st0; }
        _ctr211 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 4072 "achii.hh"

            goto _st138;
        _st138:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof138;
        st_case_138:
            if (((*(p))) == 32) {
                goto _ctr213;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _st138;
            }
            { goto _st0; }
        _ctr213 : {
#line 62 "ascii.rl"
            _flags_str = str();
        }

#line 4100 "achii.hh"

            goto _st139;
        _st139:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof139;
        st_case_139:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr215;
            }
            { goto _st0; }
        _ctr215 : {
#line 59 "ascii.rl"
            _u32 = 0;
        }

#line 4125 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 4131 "achii.hh"

            goto _st140;
        _ctr218 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 4139 "achii.hh"

            goto _st140;
        _st140:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof140;
        st_case_140:
            if (((*(p))) == 32) {
                goto _ctr217;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr218;
            }
            { goto _st0; }
        _ctr217 : {
#line 63 "ascii.rl"
            _expiration = _u32;
        }

#line 4167 "achii.hh"

            goto _st141;
        _st141:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof141;
        st_case_141:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr220;
            }
            { goto _st0; }
        _ctr224 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 4192 "achii.hh"

            goto _st142;
        _ctr220 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 4202 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 = 0;
            }

#line 4208 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 4214 "achii.hh"

            goto _st142;
        _st142:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof142;
        st_case_142:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr222;
                }
                case 32: {
                    goto _ctr223;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr224;
            }
            { goto _st0; }
        _ctr222 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 4247 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 4253 "achii.hh"

            goto _st143;
        _st143:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof143;
        st_case_143:
            if (((*(p))) == 10) {
                goto _ctr226;
            }
            { goto _st0; }
        _ctr226 : {
#line 70 "ascii.rl"
            {
                {
#line 87 "ascii.rl"

                    prepush();
                }
                _fsm_stack[_fsm_top] = 144;
                _fsm_top += 1;
                goto _st195;
            }
        }

#line 4283 "achii.hh"

            goto _st144;
        _st144:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof144;
        st_case_144:
            if (((*(p))) == 13) {
                goto _st145;
            }
            { goto _st0; }
        _st145:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof145;
        st_case_145:
            if (((*(p))) == 10) {
                goto _ctr229;
            }
            { goto _st0; }
        _ctr223 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 4325 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 4331 "achii.hh"

            goto _st146;
        _st146:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof146;
        st_case_146:
            if (((*(p))) == 110) {
                goto _st147;
            }
            { goto _st0; }
        _st147:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof147;
        st_case_147:
            if (((*(p))) == 111) {
                goto _st148;
            }
            { goto _st0; }
        _st148:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof148;
        st_case_148:
            if (((*(p))) == 114) {
                goto _st149;
            }
            { goto _st0; }
        _st149:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof149;
        st_case_149:
            if (((*(p))) == 101) {
                goto _st150;
            }
            { goto _st0; }
        _st150:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof150;
        st_case_150:
            if (((*(p))) == 112) {
                goto _st151;
            }
            { goto _st0; }
        _st151:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof151;
        st_case_151:
            if (((*(p))) == 108) {
                goto _st152;
            }
            { goto _st0; }
        _st152:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof152;
        st_case_152:
            if (((*(p))) == 121) {
                goto _ctr237;
            }
            { goto _st0; }
        _ctr237 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 4458 "achii.hh"

            goto _st153;
        _st153:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof153;
        st_case_153:
            if (((*(p))) == 13) {
                goto _st143;
            }
            { goto _st0; }
        _st154:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof154;
        st_case_154:
            switch (((*(p)))) {
                case 101: {
                    goto _st155;
                }
                case 116: {
                    goto _st176;
                }
            }
            { goto _st0; }
        _st155:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof155;
        st_case_155:
            if (((*(p))) == 116) {
                goto _st156;
            }
            { goto _st0; }
        _st156:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof156;
        st_case_156:
            if (((*(p))) == 32) {
                goto _st157;
            }
            { goto _st0; }
        _st157:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof157;
        st_case_157:
            if (((*(p))) == 32) {
                goto _st0;
            }
            { goto _ctr243; }
        _ctr243 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 4558 "achii.hh"

            goto _st158;
        _st158:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof158;
        st_case_158:
            if (((*(p))) == 32) {
                goto _ctr245;
            }
            { goto _st158; }
        _ctr245 : {
#line 61 "ascii.rl"
            _key = memcache::item_key(str());
        }

#line 4583 "achii.hh"

            goto _st159;
        _st159:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof159;
        st_case_159:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr247;
            }
            { goto _st0; }
        _ctr247 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 4610 "achii.hh"

            goto _st160;
        _st160:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof160;
        st_case_160:
            if (((*(p))) == 32) {
                goto _ctr249;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _st160;
            }
            { goto _st0; }
        _ctr249 : {
#line 62 "ascii.rl"
            _flags_str = str();
        }

#line 4638 "achii.hh"

            goto _st161;
        _st161:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof161;
        st_case_161:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr251;
            }
            { goto _st0; }
        _ctr251 : {
#line 59 "ascii.rl"
            _u32 = 0;
        }

#line 4663 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 4669 "achii.hh"

            goto _st162;
        _ctr254 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 4677 "achii.hh"

            goto _st162;
        _st162:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof162;
        st_case_162:
            if (((*(p))) == 32) {
                goto _ctr253;
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr254;
            }
            { goto _st0; }
        _ctr253 : {
#line 63 "ascii.rl"
            _expiration = _u32;
        }

#line 4705 "achii.hh"

            goto _st163;
        _st163:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof163;
        st_case_163:
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr256;
            }
            { goto _st0; }
        _ctr260 : {
#line 59 "ascii.rl"
            _u32 *= 10;
            _u32 += (((*(p)))) - '0';
        }

#line 4730 "achii.hh"

            goto _st164;
        _ctr256 : {
#line 36 "ascii.rl"

            g.mark_start(p);
        }

#line 4740 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 = 0;
            }

#line 4746 "achii.hh"

            {
#line 59 "ascii.rl"
                _u32 *= 10;
                _u32 += (((*(p)))) - '0';
            }

#line 4752 "achii.hh"

            goto _st164;
        _st164:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof164;
        st_case_164:
            switch (((*(p)))) {
                case 13: {
                    goto _ctr258;
                }
                case 32: {
                    goto _ctr259;
                }
            }
            if (48 <= ((*(p))) && ((*(p))) <= 57) {
                goto _ctr260;
            }
            { goto _st0; }
        _ctr258 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 4785 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 4791 "achii.hh"

            goto _st165;
        _st165:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof165;
        st_case_165:
            if (((*(p))) == 10) {
                goto _ctr262;
            }
            { goto _st0; }
        _ctr262 : {
#line 70 "ascii.rl"
            {
                {
#line 87 "ascii.rl"

                    prepush();
                }
                _fsm_stack[_fsm_top] = 166;
                _fsm_top += 1;
                goto _st195;
            }
        }

#line 4821 "achii.hh"

            goto _st166;
        _st166:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof166;
        st_case_166:
            if (((*(p))) == 13) {
                goto _st167;
            }
            { goto _st0; }
        _st167:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof167;
        st_case_167:
            if (((*(p))) == 10) {
                goto _ctr265;
            }
            { goto _st0; }
        _ctr259 : {
#line 64 "ascii.rl"
            _size = _u32;
            _size_str = str();
        }

#line 4863 "achii.hh"

            {
#line 66 "ascii.rl"
                _noreply = false;
            }

#line 4869 "achii.hh"

            goto _st168;
        _st168:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof168;
        st_case_168:
            if (((*(p))) == 110) {
                goto _st169;
            }
            { goto _st0; }
        _st169:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof169;
        st_case_169:
            if (((*(p))) == 111) {
                goto _st170;
            }
            { goto _st0; }
        _st170:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof170;
        st_case_170:
            if (((*(p))) == 114) {
                goto _st171;
            }
            { goto _st0; }
        _st171:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof171;
        st_case_171:
            if (((*(p))) == 101) {
                goto _st172;
            }
            { goto _st0; }
        _st172:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof172;
        st_case_172:
            if (((*(p))) == 112) {
                goto _st173;
            }
            { goto _st0; }
        _st173:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof173;
        st_case_173:
            if (((*(p))) == 108) {
                goto _st174;
            }
            { goto _st0; }
        _st174:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof174;
        st_case_174:
            if (((*(p))) == 121) {
                goto _ctr273;
            }
            { goto _st0; }
        _ctr273 : {
#line 66 "ascii.rl"
            _noreply = true;
        }

#line 4996 "achii.hh"

            goto _st175;
        _st175:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof175;
        st_case_175:
            if (((*(p))) == 13) {
                goto _st165;
            }
            { goto _st0; }
        _st176:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof176;
        st_case_176:
            if (((*(p))) == 97) {
                goto _st177;
            }
            { goto _st0; }
        _st177:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof177;
        st_case_177:
            if (((*(p))) == 116) {
                goto _st178;
            }
            { goto _st0; }
        _st178:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof178;
        st_case_178:
            if (((*(p))) == 115) {
                goto _st179;
            }
            { goto _st0; }
        _st179:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof179;
        st_case_179:
            switch (((*(p)))) {
                case 13: {
                    goto _st180;
                }
                case 32: {
                    goto _st181;
                }
            }
            { goto _st0; }
        _st180:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof180;
        st_case_180:
            if (((*(p))) == 10) {
                goto _ctr280;
            }
            { goto _st0; }
        _st181:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof181;
        st_case_181:
            if (((*(p))) == 104) {
                goto _st182;
            }
            { goto _st0; }
        _st182:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof182;
        st_case_182:
            if (((*(p))) == 97) {
                goto _st183;
            }
            { goto _st0; }
        _st183:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof183;
        st_case_183:
            if (((*(p))) == 115) {
                goto _st184;
            }
            { goto _st0; }
        _st184:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof184;
        st_case_184:
            if (((*(p))) == 104) {
                goto _st185;
            }
            { goto _st0; }
        _st185:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof185;
        st_case_185:
            if (((*(p))) == 13) {
                goto _st186;
            }
            { goto _st0; }
        _st186:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof186;
        st_case_186:
            if (((*(p))) == 10) {
                goto _ctr286;
            }
            { goto _st0; }
        _st187:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof187;
        st_case_187:
            if (((*(p))) == 101) {
                goto _st188;
            }
            { goto _st0; }
        _st188:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof188;
        st_case_188:
            if (((*(p))) == 114) {
                goto _st189;
            }
            { goto _st0; }
        _st189:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof189;
        st_case_189:
            if (((*(p))) == 115) {
                goto _st190;
            }
            { goto _st0; }
        _st190:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof190;
        st_case_190:
            if (((*(p))) == 105) {
                goto _st191;
            }
            { goto _st0; }
        _st191:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof191;
        st_case_191:
            if (((*(p))) == 111) {
                goto _st192;
            }
            { goto _st0; }
        _st192:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof192;
        st_case_192:
            if (((*(p))) == 110) {
                goto _st193;
            }
            { goto _st0; }
        _st193:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof193;
        st_case_193:
            if (((*(p))) == 13) {
                goto _st194;
            }
            { goto _st0; }
        _st194:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof194;
        st_case_194:
            if (((*(p))) == 10) {
                goto _ctr294;
            }
            { goto _st0; }
        _st195:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof195;
        st_case_195 : { goto _ctr296; }
        _ctr296 : {
#line 40 "ascii.rl"

            g.mark_start(p);
            _size_left = _size;
        }

#line 5366 "achii.hh"

            {
#line 45 "ascii.rl"

                auto len = std::min((uint32_t)(pe - p), _size_left);
                _size_left -= len;
                p += len;
                if (_size_left == 0) {
                    _blob = str();
                    p--;
                    {
                        _fsm_top -= 1;
                        _fsm_cs = _fsm_stack[_fsm_top];
                        {
#line 91 "ascii.rl"

                            postpop();
                        }
                        goto _again;
                    }
                }
                p--;
            }

#line 5387 "achii.hh"

            goto _st200;
        _ctr302 : {
#line 45 "ascii.rl"

            auto len = std::min((uint32_t)(pe - p), _size_left);
            _size_left -= len;
            p += len;
            if (_size_left == 0) {
                _blob = str();
                p--;
                {
                    _fsm_top -= 1;
                    _fsm_cs = _fsm_stack[_fsm_top];
                    {
#line 91 "ascii.rl"

                        postpop();
                    }
                    goto _again;
                }
            }
            p--;
        }

#line 5410 "achii.hh"

            goto _st200;
        _st200:
            if (p == eof) {
                if (_fsm_cs >= 196)
                    goto _out;
                else
                    goto _pop;
            }
            p += 1;
            if (p == pe)
                goto _test_eof200;
        st_case_200 : { goto _ctr302; }
        st_out:
        _test_eof1:
            _fsm_cs = 1;
            goto _test_eof;
        _test_eof2:
            _fsm_cs = 2;
            goto _test_eof;
        _test_eof3:
            _fsm_cs = 3;
            goto _test_eof;
        _test_eof4:
            _fsm_cs = 4;
            goto _test_eof;
        _test_eof5:
            _fsm_cs = 5;
            goto _test_eof;
        _test_eof6:
            _fsm_cs = 6;
            goto _test_eof;
        _test_eof7:
            _fsm_cs = 7;
            goto _test_eof;
        _test_eof8:
            _fsm_cs = 8;
            goto _test_eof;
        _test_eof9:
            _fsm_cs = 9;
            goto _test_eof;
        _test_eof10:
            _fsm_cs = 10;
            goto _test_eof;
        _test_eof11:
            _fsm_cs = 11;
            goto _test_eof;
        _test_eof12:
            _fsm_cs = 12;
            goto _test_eof;
        _test_eof13:
            _fsm_cs = 13;
            goto _test_eof;
        _test_eof14:
            _fsm_cs = 14;
            goto _test_eof;
        _test_eof15:
            _fsm_cs = 15;
            goto _test_eof;
        _test_eof196:
            _fsm_cs = 196;
            goto _test_eof;
        _test_eof16:
            _fsm_cs = 16;
            goto _test_eof;
        _test_eof17:
            _fsm_cs = 17;
            goto _test_eof;
        _test_eof18:
            _fsm_cs = 18;
            goto _test_eof;
        _test_eof19:
            _fsm_cs = 19;
            goto _test_eof;
        _test_eof20:
            _fsm_cs = 20;
            goto _test_eof;
        _test_eof21:
            _fsm_cs = 21;
            goto _test_eof;
        _test_eof22:
            _fsm_cs = 22;
            goto _test_eof;
        _test_eof23:
            _fsm_cs = 23;
            goto _test_eof;
        _test_eof24:
            _fsm_cs = 24;
            goto _test_eof;
        _test_eof25:
            _fsm_cs = 25;
            goto _test_eof;
        _test_eof26:
            _fsm_cs = 26;
            goto _test_eof;
        _test_eof27:
            _fsm_cs = 27;
            goto _test_eof;
        _test_eof28:
            _fsm_cs = 28;
            goto _test_eof;
        _test_eof29:
            _fsm_cs = 29;
            goto _test_eof;
        _test_eof30:
            _fsm_cs = 30;
            goto _test_eof;
        _test_eof31:
            _fsm_cs = 31;
            goto _test_eof;
        _test_eof32:
            _fsm_cs = 32;
            goto _test_eof;
        _test_eof33:
            _fsm_cs = 33;
            goto _test_eof;
        _test_eof34:
            _fsm_cs = 34;
            goto _test_eof;
        _test_eof35:
            _fsm_cs = 35;
            goto _test_eof;
        _test_eof36:
            _fsm_cs = 36;
            goto _test_eof;
        _test_eof37:
            _fsm_cs = 37;
            goto _test_eof;
        _test_eof38:
            _fsm_cs = 38;
            goto _test_eof;
        _test_eof39:
            _fsm_cs = 39;
            goto _test_eof;
        _test_eof40:
            _fsm_cs = 40;
            goto _test_eof;
        _test_eof41:
            _fsm_cs = 41;
            goto _test_eof;
        _test_eof42:
            _fsm_cs = 42;
            goto _test_eof;
        _test_eof43:
            _fsm_cs = 43;
            goto _test_eof;
        _test_eof44:
            _fsm_cs = 44;
            goto _test_eof;
        _test_eof45:
            _fsm_cs = 45;
            goto _test_eof;
        _test_eof46:
            _fsm_cs = 46;
            goto _test_eof;
        _test_eof47:
            _fsm_cs = 47;
            goto _test_eof;
        _test_eof48:
            _fsm_cs = 48;
            goto _test_eof;
        _test_eof49:
            _fsm_cs = 49;
            goto _test_eof;
        _test_eof50:
            _fsm_cs = 50;
            goto _test_eof;
        _test_eof51:
            _fsm_cs = 51;
            goto _test_eof;
        _test_eof52:
            _fsm_cs = 52;
            goto _test_eof;
        _test_eof53:
            _fsm_cs = 53;
            goto _test_eof;
        _test_eof54:
            _fsm_cs = 54;
            goto _test_eof;
        _test_eof55:
            _fsm_cs = 55;
            goto _test_eof;
        _test_eof56:
            _fsm_cs = 56;
            goto _test_eof;
        _test_eof57:
            _fsm_cs = 57;
            goto _test_eof;
        _test_eof58:
            _fsm_cs = 58;
            goto _test_eof;
        _test_eof59:
            _fsm_cs = 59;
            goto _test_eof;
        _test_eof60:
            _fsm_cs = 60;
            goto _test_eof;
        _test_eof61:
            _fsm_cs = 61;
            goto _test_eof;
        _test_eof62:
            _fsm_cs = 62;
            goto _test_eof;
        _test_eof63:
            _fsm_cs = 63;
            goto _test_eof;
        _test_eof64:
            _fsm_cs = 64;
            goto _test_eof;
        _test_eof65:
            _fsm_cs = 65;
            goto _test_eof;
        _test_eof66:
            _fsm_cs = 66;
            goto _test_eof;
        _test_eof67:
            _fsm_cs = 67;
            goto _test_eof;
        _test_eof68:
            _fsm_cs = 68;
            goto _test_eof;
        _test_eof69:
            _fsm_cs = 69;
            goto _test_eof;
        _test_eof70:
            _fsm_cs = 70;
            goto _test_eof;
        _test_eof71:
            _fsm_cs = 71;
            goto _test_eof;
        _test_eof197:
            _fsm_cs = 197;
            goto _test_eof;
        _test_eof72:
            _fsm_cs = 72;
            goto _test_eof;
        _test_eof73:
            _fsm_cs = 73;
            goto _test_eof;
        _test_eof74:
            _fsm_cs = 74;
            goto _test_eof;
        _test_eof75:
            _fsm_cs = 75;
            goto _test_eof;
        _test_eof76:
            _fsm_cs = 76;
            goto _test_eof;
        _test_eof77:
            _fsm_cs = 77;
            goto _test_eof;
        _test_eof78:
            _fsm_cs = 78;
            goto _test_eof;
        _test_eof79:
            _fsm_cs = 79;
            goto _test_eof;
        _test_eof80:
            _fsm_cs = 80;
            goto _test_eof;
        _test_eof81:
            _fsm_cs = 81;
            goto _test_eof;
        _test_eof82:
            _fsm_cs = 82;
            goto _test_eof;
        _test_eof83:
            _fsm_cs = 83;
            goto _test_eof;
        _test_eof84:
            _fsm_cs = 84;
            goto _test_eof;
        _test_eof85:
            _fsm_cs = 85;
            goto _test_eof;
        _test_eof86:
            _fsm_cs = 86;
            goto _test_eof;
        _test_eof87:
            _fsm_cs = 87;
            goto _test_eof;
        _test_eof88:
            _fsm_cs = 88;
            goto _test_eof;
        _test_eof89:
            _fsm_cs = 89;
            goto _test_eof;
        _test_eof90:
            _fsm_cs = 90;
            goto _test_eof;
        _test_eof91:
            _fsm_cs = 91;
            goto _test_eof;
        _test_eof92:
            _fsm_cs = 92;
            goto _test_eof;
        _test_eof93:
            _fsm_cs = 93;
            goto _test_eof;
        _test_eof94:
            _fsm_cs = 94;
            goto _test_eof;
        _test_eof95:
            _fsm_cs = 95;
            goto _test_eof;
        _test_eof96:
            _fsm_cs = 96;
            goto _test_eof;
        _test_eof97:
            _fsm_cs = 97;
            goto _test_eof;
        _test_eof98:
            _fsm_cs = 98;
            goto _test_eof;
        _test_eof99:
            _fsm_cs = 99;
            goto _test_eof;
        _test_eof100:
            _fsm_cs = 100;
            goto _test_eof;
        _test_eof101:
            _fsm_cs = 101;
            goto _test_eof;
        _test_eof102:
            _fsm_cs = 102;
            goto _test_eof;
        _test_eof103:
            _fsm_cs = 103;
            goto _test_eof;
        _test_eof104:
            _fsm_cs = 104;
            goto _test_eof;
        _test_eof105:
            _fsm_cs = 105;
            goto _test_eof;
        _test_eof106:
            _fsm_cs = 106;
            goto _test_eof;
        _test_eof198:
            _fsm_cs = 198;
            goto _test_eof;
        _test_eof107:
            _fsm_cs = 107;
            goto _test_eof;
        _test_eof108:
            _fsm_cs = 108;
            goto _test_eof;
        _test_eof109:
            _fsm_cs = 109;
            goto _test_eof;
        _test_eof110:
            _fsm_cs = 110;
            goto _test_eof;
        _test_eof199:
            _fsm_cs = 199;
            goto _test_eof;
        _test_eof111:
            _fsm_cs = 111;
            goto _test_eof;
        _test_eof112:
            _fsm_cs = 112;
            goto _test_eof;
        _test_eof113:
            _fsm_cs = 113;
            goto _test_eof;
        _test_eof114:
            _fsm_cs = 114;
            goto _test_eof;
        _test_eof115:
            _fsm_cs = 115;
            goto _test_eof;
        _test_eof116:
            _fsm_cs = 116;
            goto _test_eof;
        _test_eof117:
            _fsm_cs = 117;
            goto _test_eof;
        _test_eof118:
            _fsm_cs = 118;
            goto _test_eof;
        _test_eof119:
            _fsm_cs = 119;
            goto _test_eof;
        _test_eof120:
            _fsm_cs = 120;
            goto _test_eof;
        _test_eof121:
            _fsm_cs = 121;
            goto _test_eof;
        _test_eof122:
            _fsm_cs = 122;
            goto _test_eof;
        _test_eof123:
            _fsm_cs = 123;
            goto _test_eof;
        _test_eof124:
            _fsm_cs = 124;
            goto _test_eof;
        _test_eof125:
            _fsm_cs = 125;
            goto _test_eof;
        _test_eof126:
            _fsm_cs = 126;
            goto _test_eof;
        _test_eof127:
            _fsm_cs = 127;
            goto _test_eof;
        _test_eof128:
            _fsm_cs = 128;
            goto _test_eof;
        _test_eof129:
            _fsm_cs = 129;
            goto _test_eof;
        _test_eof130:
            _fsm_cs = 130;
            goto _test_eof;
        _test_eof131:
            _fsm_cs = 131;
            goto _test_eof;
        _test_eof132:
            _fsm_cs = 132;
            goto _test_eof;
        _test_eof133:
            _fsm_cs = 133;
            goto _test_eof;
        _test_eof134:
            _fsm_cs = 134;
            goto _test_eof;
        _test_eof135:
            _fsm_cs = 135;
            goto _test_eof;
        _test_eof136:
            _fsm_cs = 136;
            goto _test_eof;
        _test_eof137:
            _fsm_cs = 137;
            goto _test_eof;
        _test_eof138:
            _fsm_cs = 138;
            goto _test_eof;
        _test_eof139:
            _fsm_cs = 139;
            goto _test_eof;
        _test_eof140:
            _fsm_cs = 140;
            goto _test_eof;
        _test_eof141:
            _fsm_cs = 141;
            goto _test_eof;
        _test_eof142:
            _fsm_cs = 142;
            goto _test_eof;
        _test_eof143:
            _fsm_cs = 143;
            goto _test_eof;
        _test_eof144:
            _fsm_cs = 144;
            goto _test_eof;
        _test_eof145:
            _fsm_cs = 145;
            goto _test_eof;
        _test_eof146:
            _fsm_cs = 146;
            goto _test_eof;
        _test_eof147:
            _fsm_cs = 147;
            goto _test_eof;
        _test_eof148:
            _fsm_cs = 148;
            goto _test_eof;
        _test_eof149:
            _fsm_cs = 149;
            goto _test_eof;
        _test_eof150:
            _fsm_cs = 150;
            goto _test_eof;
        _test_eof151:
            _fsm_cs = 151;
            goto _test_eof;
        _test_eof152:
            _fsm_cs = 152;
            goto _test_eof;
        _test_eof153:
            _fsm_cs = 153;
            goto _test_eof;
        _test_eof154:
            _fsm_cs = 154;
            goto _test_eof;
        _test_eof155:
            _fsm_cs = 155;
            goto _test_eof;
        _test_eof156:
            _fsm_cs = 156;
            goto _test_eof;
        _test_eof157:
            _fsm_cs = 157;
            goto _test_eof;
        _test_eof158:
            _fsm_cs = 158;
            goto _test_eof;
        _test_eof159:
            _fsm_cs = 159;
            goto _test_eof;
        _test_eof160:
            _fsm_cs = 160;
            goto _test_eof;
        _test_eof161:
            _fsm_cs = 161;
            goto _test_eof;
        _test_eof162:
            _fsm_cs = 162;
            goto _test_eof;
        _test_eof163:
            _fsm_cs = 163;
            goto _test_eof;
        _test_eof164:
            _fsm_cs = 164;
            goto _test_eof;
        _test_eof165:
            _fsm_cs = 165;
            goto _test_eof;
        _test_eof166:
            _fsm_cs = 166;
            goto _test_eof;
        _test_eof167:
            _fsm_cs = 167;
            goto _test_eof;
        _test_eof168:
            _fsm_cs = 168;
            goto _test_eof;
        _test_eof169:
            _fsm_cs = 169;
            goto _test_eof;
        _test_eof170:
            _fsm_cs = 170;
            goto _test_eof;
        _test_eof171:
            _fsm_cs = 171;
            goto _test_eof;
        _test_eof172:
            _fsm_cs = 172;
            goto _test_eof;
        _test_eof173:
            _fsm_cs = 173;
            goto _test_eof;
        _test_eof174:
            _fsm_cs = 174;
            goto _test_eof;
        _test_eof175:
            _fsm_cs = 175;
            goto _test_eof;
        _test_eof176:
            _fsm_cs = 176;
            goto _test_eof;
        _test_eof177:
            _fsm_cs = 177;
            goto _test_eof;
        _test_eof178:
            _fsm_cs = 178;
            goto _test_eof;
        _test_eof179:
            _fsm_cs = 179;
            goto _test_eof;
        _test_eof180:
            _fsm_cs = 180;
            goto _test_eof;
        _test_eof181:
            _fsm_cs = 181;
            goto _test_eof;
        _test_eof182:
            _fsm_cs = 182;
            goto _test_eof;
        _test_eof183:
            _fsm_cs = 183;
            goto _test_eof;
        _test_eof184:
            _fsm_cs = 184;
            goto _test_eof;
        _test_eof185:
            _fsm_cs = 185;
            goto _test_eof;
        _test_eof186:
            _fsm_cs = 186;
            goto _test_eof;
        _test_eof187:
            _fsm_cs = 187;
            goto _test_eof;
        _test_eof188:
            _fsm_cs = 188;
            goto _test_eof;
        _test_eof189:
            _fsm_cs = 189;
            goto _test_eof;
        _test_eof190:
            _fsm_cs = 190;
            goto _test_eof;
        _test_eof191:
            _fsm_cs = 191;
            goto _test_eof;
        _test_eof192:
            _fsm_cs = 192;
            goto _test_eof;
        _test_eof193:
            _fsm_cs = 193;
            goto _test_eof;
        _test_eof194:
            _fsm_cs = 194;
            goto _test_eof;
        _test_eof195:
            _fsm_cs = 195;
            goto _test_eof;
        _test_eof200:
            _fsm_cs = 200;
            goto _test_eof;

        _test_eof : { }
            if (p == eof) {
                switch (_fsm_cs) {
                    case 1: {
                        break;
                    }
                    case 0: {
                        break;
                    }
                    case 2: {
                        break;
                    }
                    case 3: {
                        break;
                    }
                    case 4: {
                        break;
                    }
                    case 5: {
                        break;
                    }
                    case 6: {
                        break;
                    }
                    case 7: {
                        break;
                    }
                    case 8: {
                        break;
                    }
                    case 9: {
                        break;
                    }
                    case 10: {
                        break;
                    }
                    case 11: {
                        break;
                    }
                    case 12: {
                        break;
                    }
                    case 13: {
                        break;
                    }
                    case 14: {
                        break;
                    }
                    case 15: {
                        break;
                    }
                    case 196: {
                        break;
                    }
                    case 16: {
                        break;
                    }
                    case 17: {
                        break;
                    }
                    case 18: {
                        break;
                    }
                    case 19: {
                        break;
                    }
                    case 20: {
                        break;
                    }
                    case 21: {
                        break;
                    }
                    case 22: {
                        break;
                    }
                    case 23: {
                        break;
                    }
                    case 24: {
                        break;
                    }
                    case 25: {
                        break;
                    }
                    case 26: {
                        break;
                    }
                    case 27: {
                        break;
                    }
                    case 28: {
                        break;
                    }
                    case 29: {
                        break;
                    }
                    case 30: {
                        break;
                    }
                    case 31: {
                        break;
                    }
                    case 32: {
                        break;
                    }
                    case 33: {
                        break;
                    }
                    case 34: {
                        break;
                    }
                    case 35: {
                        break;
                    }
                    case 36: {
                        break;
                    }
                    case 37: {
                        break;
                    }
                    case 38: {
                        break;
                    }
                    case 39: {
                        break;
                    }
                    case 40: {
                        break;
                    }
                    case 41: {
                        break;
                    }
                    case 42: {
                        break;
                    }
                    case 43: {
                        break;
                    }
                    case 44: {
                        break;
                    }
                    case 45: {
                        break;
                    }
                    case 46: {
                        break;
                    }
                    case 47: {
                        break;
                    }
                    case 48: {
                        break;
                    }
                    case 49: {
                        break;
                    }
                    case 50: {
                        break;
                    }
                    case 51: {
                        break;
                    }
                    case 52: {
                        break;
                    }
                    case 53: {
                        break;
                    }
                    case 54: {
                        break;
                    }
                    case 55: {
                        break;
                    }
                    case 56: {
                        break;
                    }
                    case 57: {
                        break;
                    }
                    case 58: {
                        break;
                    }
                    case 59: {
                        break;
                    }
                    case 60: {
                        break;
                    }
                    case 61: {
                        break;
                    }
                    case 62: {
                        break;
                    }
                    case 63: {
                        break;
                    }
                    case 64: {
                        break;
                    }
                    case 65: {
                        break;
                    }
                    case 66: {
                        break;
                    }
                    case 67: {
                        break;
                    }
                    case 68: {
                        break;
                    }
                    case 69: {
                        break;
                    }
                    case 70: {
                        break;
                    }
                    case 71: {
                        break;
                    }
                    case 197: {
                        break;
                    }
                    case 72: {
                        break;
                    }
                    case 73: {
                        break;
                    }
                    case 74: {
                        break;
                    }
                    case 75: {
                        break;
                    }
                    case 76: {
                        break;
                    }
                    case 77: {
                        break;
                    }
                    case 78: {
                        break;
                    }
                    case 79: {
                        break;
                    }
                    case 80: {
                        break;
                    }
                    case 81: {
                        break;
                    }
                    case 82: {
                        break;
                    }
                    case 83: {
                        break;
                    }
                    case 84: {
                        break;
                    }
                    case 85: {
                        break;
                    }
                    case 86: {
                        break;
                    }
                    case 87: {
                        break;
                    }
                    case 88: {
                        break;
                    }
                    case 89: {
                        break;
                    }
                    case 90: {
                        break;
                    }
                    case 91: {
                        break;
                    }
                    case 92: {
                        break;
                    }
                    case 93: {
                        break;
                    }
                    case 94: {
                        break;
                    }
                    case 95: {
                        break;
                    }
                    case 96: {
                        break;
                    }
                    case 97: {
                        break;
                    }
                    case 98: {
                        break;
                    }
                    case 99: {
                        break;
                    }
                    case 100: {
                        break;
                    }
                    case 101: {
                        break;
                    }
                    case 102: {
                        break;
                    }
                    case 103: {
                        break;
                    }
                    case 104: {
                        break;
                    }
                    case 105: {
                        break;
                    }
                    case 106: {
                        break;
                    }
                    case 198: {
                        break;
                    }
                    case 107: {
                        break;
                    }
                    case 108: {
                        break;
                    }
                    case 109: {
                        break;
                    }
                    case 110: {
                        break;
                    }
                    case 199: {
                        break;
                    }
                    case 111: {
                        break;
                    }
                    case 112: {
                        break;
                    }
                    case 113: {
                        break;
                    }
                    case 114: {
                        break;
                    }
                    case 115: {
                        break;
                    }
                    case 116: {
                        break;
                    }
                    case 117: {
                        break;
                    }
                    case 118: {
                        break;
                    }
                    case 119: {
                        break;
                    }
                    case 120: {
                        break;
                    }
                    case 121: {
                        break;
                    }
                    case 122: {
                        break;
                    }
                    case 123: {
                        break;
                    }
                    case 124: {
                        break;
                    }
                    case 125: {
                        break;
                    }
                    case 126: {
                        break;
                    }
                    case 127: {
                        break;
                    }
                    case 128: {
                        break;
                    }
                    case 129: {
                        break;
                    }
                    case 130: {
                        break;
                    }
                    case 131: {
                        break;
                    }
                    case 132: {
                        break;
                    }
                    case 133: {
                        break;
                    }
                    case 134: {
                        break;
                    }
                    case 135: {
                        break;
                    }
                    case 136: {
                        break;
                    }
                    case 137: {
                        break;
                    }
                    case 138: {
                        break;
                    }
                    case 139: {
                        break;
                    }
                    case 140: {
                        break;
                    }
                    case 141: {
                        break;
                    }
                    case 142: {
                        break;
                    }
                    case 143: {
                        break;
                    }
                    case 144: {
                        break;
                    }
                    case 145: {
                        break;
                    }
                    case 146: {
                        break;
                    }
                    case 147: {
                        break;
                    }
                    case 148: {
                        break;
                    }
                    case 149: {
                        break;
                    }
                    case 150: {
                        break;
                    }
                    case 151: {
                        break;
                    }
                    case 152: {
                        break;
                    }
                    case 153: {
                        break;
                    }
                    case 154: {
                        break;
                    }
                    case 155: {
                        break;
                    }
                    case 156: {
                        break;
                    }
                    case 157: {
                        break;
                    }
                    case 158: {
                        break;
                    }
                    case 159: {
                        break;
                    }
                    case 160: {
                        break;
                    }
                    case 161: {
                        break;
                    }
                    case 162: {
                        break;
                    }
                    case 163: {
                        break;
                    }
                    case 164: {
                        break;
                    }
                    case 165: {
                        break;
                    }
                    case 166: {
                        break;
                    }
                    case 167: {
                        break;
                    }
                    case 168: {
                        break;
                    }
                    case 169: {
                        break;
                    }
                    case 170: {
                        break;
                    }
                    case 171: {
                        break;
                    }
                    case 172: {
                        break;
                    }
                    case 173: {
                        break;
                    }
                    case 174: {
                        break;
                    }
                    case 175: {
                        break;
                    }
                    case 176: {
                        break;
                    }
                    case 177: {
                        break;
                    }
                    case 178: {
                        break;
                    }
                    case 179: {
                        break;
                    }
                    case 180: {
                        break;
                    }
                    case 181: {
                        break;
                    }
                    case 182: {
                        break;
                    }
                    case 183: {
                        break;
                    }
                    case 184: {
                        break;
                    }
                    case 185: {
                        break;
                    }
                    case 186: {
                        break;
                    }
                    case 187: {
                        break;
                    }
                    case 188: {
                        break;
                    }
                    case 189: {
                        break;
                    }
                    case 190: {
                        break;
                    }
                    case 191: {
                        break;
                    }
                    case 192: {
                        break;
                    }
                    case 193: {
                        break;
                    }
                    case 194: {
                        break;
                    }
                    case 195: {
                        break;
                    }
                    case 200: {
                        break;
                    }
                }
                switch (_fsm_cs) { }
                switch (_fsm_cs) {
                    case 1:
                        goto _ctr1;
                    case 0:
                        goto _st0;
                    case 2:
                        goto _st2;
                    case 3:
                        goto _st3;
                    case 4:
                        goto _st4;
                    case 5:
                        goto _st5;
                    case 6:
                        goto _st6;
                    case 7:
                        goto _st7;
                    case 8:
                        goto _st8;
                    case 9:
                        goto _st9;
                    case 10:
                        goto _st10;
                    case 11:
                        goto _st11;
                    case 12:
                        goto _st12;
                    case 13:
                        goto _st13;
                    case 14:
                        goto _st14;
                    case 15:
                        goto _st15;
                    case 196:
                        goto _st196;
                    case 16:
                        goto _st16;
                    case 17:
                        goto _st17;
                    case 18:
                        goto _st18;
                    case 19:
                        goto _st19;
                    case 20:
                        goto _st20;
                    case 21:
                        goto _st21;
                    case 22:
                        goto _st22;
                    case 23:
                        goto _st23;
                    case 24:
                        goto _st24;
                    case 25:
                        goto _st25;
                    case 26:
                        goto _st26;
                    case 27:
                        goto _st27;
                    case 28:
                        goto _st28;
                    case 29:
                        goto _st29;
                    case 30:
                        goto _st30;
                    case 31:
                        goto _st31;
                    case 32:
                        goto _st32;
                    case 33:
                        goto _st33;
                    case 34:
                        goto _st34;
                    case 35:
                        goto _st35;
                    case 36:
                        goto _st36;
                    case 37:
                        goto _st37;
                    case 38:
                        goto _st38;
                    case 39:
                        goto _st39;
                    case 40:
                        goto _st40;
                    case 41:
                        goto _st41;
                    case 42:
                        goto _st42;
                    case 43:
                        goto _st43;
                    case 44:
                        goto _st44;
                    case 45:
                        goto _st45;
                    case 46:
                        goto _st46;
                    case 47:
                        goto _st47;
                    case 48:
                        goto _st48;
                    case 49:
                        goto _st49;
                    case 50:
                        goto _st50;
                    case 51:
                        goto _st51;
                    case 52:
                        goto _st52;
                    case 53:
                        goto _st53;
                    case 54:
                        goto _st54;
                    case 55:
                        goto _st55;
                    case 56:
                        goto _st56;
                    case 57:
                        goto _st57;
                    case 58:
                        goto _st58;
                    case 59:
                        goto _st59;
                    case 60:
                        goto _st60;
                    case 61:
                        goto _st61;
                    case 62:
                        goto _st62;
                    case 63:
                        goto _st63;
                    case 64:
                        goto _st64;
                    case 65:
                        goto _st65;
                    case 66:
                        goto _st66;
                    case 67:
                        goto _st67;
                    case 68:
                        goto _st68;
                    case 69:
                        goto _st69;
                    case 70:
                        goto _st70;
                    case 71:
                        goto _st71;
                    case 197:
                        goto _st197;
                    case 72:
                        goto _st72;
                    case 73:
                        goto _st73;
                    case 74:
                        goto _st74;
                    case 75:
                        goto _st75;
                    case 76:
                        goto _st76;
                    case 77:
                        goto _st77;
                    case 78:
                        goto _st78;
                    case 79:
                        goto _st79;
                    case 80:
                        goto _st80;
                    case 81:
                        goto _st81;
                    case 82:
                        goto _st82;
                    case 83:
                        goto _st83;
                    case 84:
                        goto _st84;
                    case 85:
                        goto _st85;
                    case 86:
                        goto _st86;
                    case 87:
                        goto _st87;
                    case 88:
                        goto _st88;
                    case 89:
                        goto _st89;
                    case 90:
                        goto _st90;
                    case 91:
                        goto _st91;
                    case 92:
                        goto _st92;
                    case 93:
                        goto _st93;
                    case 94:
                        goto _st94;
                    case 95:
                        goto _st95;
                    case 96:
                        goto _st96;
                    case 97:
                        goto _st97;
                    case 98:
                        goto _st98;
                    case 99:
                        goto _st99;
                    case 100:
                        goto _st100;
                    case 101:
                        goto _st101;
                    case 102:
                        goto _st102;
                    case 103:
                        goto _st103;
                    case 104:
                        goto _st104;
                    case 105:
                        goto _st105;
                    case 106:
                        goto _st106;
                    case 198:
                        goto _st198;
                    case 107:
                        goto _st107;
                    case 108:
                        goto _st108;
                    case 109:
                        goto _st109;
                    case 110:
                        goto _st110;
                    case 199:
                        goto _st199;
                    case 111:
                        goto _st111;
                    case 112:
                        goto _st112;
                    case 113:
                        goto _st113;
                    case 114:
                        goto _st114;
                    case 115:
                        goto _st115;
                    case 116:
                        goto _st116;
                    case 117:
                        goto _st117;
                    case 118:
                        goto _st118;
                    case 119:
                        goto _st119;
                    case 120:
                        goto _st120;
                    case 121:
                        goto _st121;
                    case 122:
                        goto _st122;
                    case 123:
                        goto _st123;
                    case 124:
                        goto _st124;
                    case 125:
                        goto _st125;
                    case 126:
                        goto _st126;
                    case 127:
                        goto _st127;
                    case 128:
                        goto _st128;
                    case 129:
                        goto _st129;
                    case 130:
                        goto _st130;
                    case 131:
                        goto _st131;
                    case 132:
                        goto _st132;
                    case 133:
                        goto _st133;
                    case 134:
                        goto _st134;
                    case 135:
                        goto _st135;
                    case 136:
                        goto _st136;
                    case 137:
                        goto _st137;
                    case 138:
                        goto _st138;
                    case 139:
                        goto _st139;
                    case 140:
                        goto _st140;
                    case 141:
                        goto _st141;
                    case 142:
                        goto _st142;
                    case 143:
                        goto _st143;
                    case 144:
                        goto _st144;
                    case 145:
                        goto _st145;
                    case 146:
                        goto _st146;
                    case 147:
                        goto _st147;
                    case 148:
                        goto _st148;
                    case 149:
                        goto _st149;
                    case 150:
                        goto _st150;
                    case 151:
                        goto _st151;
                    case 152:
                        goto _st152;
                    case 153:
                        goto _st153;
                    case 154:
                        goto _st154;
                    case 155:
                        goto _st155;
                    case 156:
                        goto _st156;
                    case 157:
                        goto _st157;
                    case 158:
                        goto _st158;
                    case 159:
                        goto _st159;
                    case 160:
                        goto _st160;
                    case 161:
                        goto _st161;
                    case 162:
                        goto _st162;
                    case 163:
                        goto _st163;
                    case 164:
                        goto _st164;
                    case 165:
                        goto _st165;
                    case 166:
                        goto _st166;
                    case 167:
                        goto _st167;
                    case 168:
                        goto _st168;
                    case 169:
                        goto _st169;
                    case 170:
                        goto _st170;
                    case 171:
                        goto _st171;
                    case 172:
                        goto _st172;
                    case 173:
                        goto _st173;
                    case 174:
                        goto _st174;
                    case 175:
                        goto _st175;
                    case 176:
                        goto _st176;
                    case 177:
                        goto _st177;
                    case 178:
                        goto _st178;
                    case 179:
                        goto _st179;
                    case 180:
                        goto _st180;
                    case 181:
                        goto _st181;
                    case 182:
                        goto _st182;
                    case 183:
                        goto _st183;
                    case 184:
                        goto _st184;
                    case 185:
                        goto _st185;
                    case 186:
                        goto _st186;
                    case 187:
                        goto _st187;
                    case 188:
                        goto _st188;
                    case 189:
                        goto _st189;
                    case 190:
                        goto _st190;
                    case 191:
                        goto _st191;
                    case 192:
                        goto _st192;
                    case 193:
                        goto _st193;
                    case 194:
                        goto _st194;
                    case 195:
                        goto _st195;
                    case 200:
                        goto _st200;
                }
            }

            if (_fsm_cs >= 196)
                goto _out;
        _pop : { }
        _out : { }
        }

#line 145 "ascii.rl"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
        if (_state != state::error) {
            return p;
        }
        if (p != pe) {
            p = pe;
            return p;
        }
        return nullptr;
    }
    bool eof() const {
        return _state == state::eof;
    }
};
