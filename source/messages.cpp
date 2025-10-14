//
// Created by thoma on 19/04/2025.
//


#include "messages.h"

M_ifc_init *M_ifc_init::createCopy(M_ifc_init *original) {
    if (original == nullptr) {
        return nullptr;
    }

    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Starting M_ifc_init");

    char *tmp;
    auto *return_value = new M_ifc_init();

    // Copy stops (register definition) directory string, if this path is set
    if (original->_stops == nullptr) {
        return_value->_stops = nullptr;
    } else {
        tmp = new char[strlen(original->_stops)];
        strcpy(tmp, original->_stops);
        return_value->_stops = tmp;
    }
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied stops directory");

    // Copy wave file storage directory, if this path is set
    if (original->_waves == nullptr) {
        return_value->_waves = nullptr;
    } else {
        tmp = new char[strlen(original->_waves)];
        strcpy(tmp, original->_waves);
        return_value->_waves = tmp;
    }
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied waves directory");

    // Copy instrument definition and preset directory, if this path is set
    if (original->_instr == nullptr) {
        return_value->_instr = nullptr;
    } else {
        tmp = new char[strlen(original->_instr)];
        strcpy(tmp, original->_instr);
        return_value->_instr = tmp;
    }
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied preset directory");



    // Copy short app name, if set
    if (original->_appid == nullptr) {
        return_value->_appid = nullptr;
    } else {
        tmp = new char[strlen(original->_appid)];
        strcpy(tmp, original->_appid);
        return_value->_appid = tmp;
    }

    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied appid");



    // Copy int values
    return_value->_client = original->_client;
    return_value->_ipport = original->_ipport;
    return_value->_nasect = original->_nasect; // audio sections for signal treatment
    return_value->_nkeybd = original->_nkeybd;
    return_value->_ndivis = original->_ndivis; // divisions with the rankwaves in it
    return_value->_ngroup = original->_ngroup; // a bit the same as divisions?
    return_value->_ntempe = original->_ntempe; // tuning temperaments
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied ints");

    for (int i = 0; i < NKEYBD; i++) {
        if (original->_keybdd[i]._label == nullptr) {
            return_value->_keybdd[i]._label = nullptr;
        } else {
            tmp = new char[strlen(original->_keybdd[i]._label)];
            strcpy(tmp, original->_keybdd[i]._label);
            return_value->_keybdd[i]._label = tmp;
        }

        return_value->_keybdd[i]._flags = original->_keybdd[i]._flags;
    }
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied keyboards");
    for (int i = 0; i < NDIVIS; i++) {
        if (original->_divisd[i]._label == nullptr) {
            return_value->_divisd[i]._label = nullptr;
        } else {
            tmp = new char[strlen(original->_divisd[i]._label)];
            strcpy(tmp, original->_divisd[i]._label);
            return_value->_divisd[i]._label = tmp;
        }

        return_value->_divisd[i]._asect = original->_divisd[i]._asect;
        return_value->_divisd[i]._flags = original->_divisd[i]._flags;
    }
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied divisions");
    for (int i = 0; i < 8; i++) {
        if (original->_groupd[i]._label == nullptr) {
            return_value->_groupd[i]._label = nullptr;
        } else {
            tmp = new char[strlen(original->_groupd[i]._label)];
            strcpy(tmp, original->_groupd[i]._label);
            return_value->_groupd[i]._label = tmp;
        }
        return_value->_groupd[i]._nifelm = original->_groupd[i]._nifelm;

        for (int j = 0; j < return_value->_groupd[i]._nifelm; j++) {
            if (original->_groupd[i]._ifelmd[j]._label == nullptr) {
                return_value->_groupd[i]._ifelmd[j]._label = nullptr;
            } else {
                tmp = new char[strlen(original->_groupd[i]._ifelmd[j]._label)];
                strcpy(tmp, original->_groupd[i]._ifelmd[j]._label);
                return_value->_groupd[i]._ifelmd[j]._label = tmp;

            }

            if (original->_groupd[i]._ifelmd[j]._mnemo == nullptr) {
                return_value->_groupd[i]._ifelmd[j]._mnemo = nullptr;
            } else {
                tmp = new char[strlen(original->_groupd[i]._ifelmd[j]._mnemo)];
                strcpy(tmp, original->_groupd[i]._ifelmd[j]._mnemo);
                return_value->_groupd[i]._ifelmd[j]._mnemo = tmp;
            }

            return_value->_groupd[i]._ifelmd[j]._type = original->_groupd[i]._ifelmd[j]._type;
        };

    }
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied groups");
    // Copy temperaments
    for (int i = 0; i < return_value->_ntempe; i++) {
        if (original->_temped[i]._label == nullptr) {
            return_value->_temped[i]._label = nullptr;
        } else {
            tmp = new char[strlen(original->_temped[i]._label)];
            strcpy(tmp, original->_temped[i]._label);
            return_value->_temped[i]._label = tmp;

        }
        if (original->_temped[i]._mnemo == nullptr) {
            return_value->_temped[i]._mnemo = nullptr;
        } else {
            tmp = new char[strlen(original->_temped[i]._mnemo)];
            strcpy(tmp, original->_temped[i]._mnemo);
            return_value->_temped[i]._mnemo = tmp;
        }

    }

    __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                        "Aeolus Messages M_ifc_init", "Copied temperaments");

    return return_value;


}


M_audio_info *M_audio_info::createCopy(M_audio_info *original) {
    if (original == nullptr) {
        return nullptr;
    }
    auto *return_value = new M_audio_info ();
    return_value->_nasect = original->_nasect;
    return_value->_fsamp  = original->_fsamp;
    return_value->_fsize  = original->_fsize;
    // The purpose of the _instrpar field is to transmit the corresponding
    // AeolusAudio::_audiopar array. This array has a length of 4 and holds
    // the four elements for the enum { VOLUME, REVSIZE, REVTIME, STPOSIT } defined as
    // indexes in audio.h. This field is synchronized between model and audio by the use
    // of the same pointer
    if( original->_instrpar == nullptr)
    {
        return_value->_instrpar=nullptr;
    } else {
        return_value->_instrpar=original->_instrpar;

    }

    // The same holds for the audio section parameters, which are also shared via
    // a common pointer array
        for (int i = 0; i < return_value->_nasect; i++){
            return_value->_asectpar[i] = original->_asectpar[i];
        }



    return return_value;
}

M_midi_info *M_midi_info::createCopy(M_midi_info *original) {
    if (original == nullptr) {
        return nullptr;
    }
    auto *return_value = new M_midi_info ();
    return_value ->_client = original->_client;
    return_value ->_ipport = original->_ipport;

    return_value->_chbits=original->_chbits;

    return return_value;
}

M_midi_info::M_midi_info() : ITC_mesg (MT_MIDI_INFO), _client(0),_ipport(0){
    _chbits=nullptr;
}



