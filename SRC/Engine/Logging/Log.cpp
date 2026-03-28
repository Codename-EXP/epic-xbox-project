#include "Log.h"
#include "../Graphics/Graphics.h"

const int max_logs = 16;

struct log_entry {
	float color[3];
	LPDIRECT3DVERTEXBUFFER8 vertex_buffer_handle;
	UINT vertex_count;
};
log_entry logs[max_logs];
UINT active_log_count = 0;


LPDIRECT3DVERTEXBUFFER8 ConstructStringBuffer(const char* string, UINT* output_vertex_count, bool display_extra_as_int, void* extra_to_display) {
    if (!string) return 0;

    char buffer[512];
    UINT output_char_index = 0; 
    UINT string_index = 0; // breaks at null terminator
    UINT buffer_index = 0; // auto breaks at index 512

    bool has_interpretted_extra = false;
    bool is_processing_int = false;

    while (true) {
        char c;
        if (!is_processing_int) {

            c = string[string_index];
            string_index += 1;
            // end of string
            if (!c) {
                // break if we reach end of string after processing second strng
                if (has_interpretted_extra) break;
                has_interpretted_extra = true;
                // if we set append mode, load the extra content as a str pointer
                if (!display_extra_as_int) {
                    // if no additional string provided, skip
                    if (!extra_to_display) break;

                    // load space bar as joining letter
                    string_index = 0;
                    string = (const char*)extra_to_display;
                    if (!*string) break;
                    c = ' ';
                }
                else {
                    string_index = 28;
                    is_processing_int = true;
                    continue;
                }

            }

            // check if character falls under alphanumeric patterns
            if (c >= 0x30) {
                // push number sequences into glyph indexes
                if (c < 0x3A) {
                    c -= 22;
                    goto submit;
                }
                // case insensitive check for alphabet
                else {
                    char lower_case = c & ~(char)0x20;
                    if (lower_case >= 0x41 && lower_case < 0x5B) {
                        c = lower_case - 0x41;
                        goto submit;
                    }
                }
                
                  // broken chars: _:@[]=
                // 0x3a : 
                // 0x3d =
                // 0x5f _ 
                // UNK  @ 
                // 0x5b [
                // 0x5d ]
            }
            // otherwise do a switch to convert char index in place
            switch (c) {
                case 0x20: // space 
                    goto skip;
                case 0x21: // !
                    c = 43;
                    break;
                case 0x22: // "
                    c = 42;
                    break;
                case 0x28: // (
                case 0x5b: // [
                    c = 40;
                    break;
                case 0x29: // )
                case 0x5d: // ]
                    c = 41;
                    break;
                case 0x2a: // *
                    c = 47;
                    break;
                case 0x2b: // +
                    c = 45;
                    break;
                case 0x2d: // -
                    c = 44;
                    break;
                case 0x2c: // ,
                case 0x2e: // .
                    c = 36;
                    break;
                case 0x3a: // :
                    c = 38;
                    break;
                case 0x3d: // =
                    c = 46;
                    break;
                case 0x5f: // _
                    c = 37;
                    break;
                default: // any unsupported characters
                    c = 39;
                    break;
            }
        }
        // process integer
        else {
            if (string_index == -4) break;
            // take 4 highest bits 
             UINT val = (UINT)extra_to_display;
             val >>= string_index;
             val &= 15;

             // if numeric, then bump up to glyph index 26
             if (val < 10) {
                 val += 26;
             }
             // else convert into alphabetic glyph, ABCDEF -> 012345
             else {
                 val -= 10;
             }
             c = val;

             string_index -= 4;
        }

    submit:
        // create for vertices
        buffer[buffer_index]     = (char)output_char_index +1 | 0b0'0000000;
        buffer[buffer_index + 1] = c | 0b0'0000000;
        buffer[buffer_index + 2] = (char)output_char_index +1 | 0b0'0000000;
        buffer[buffer_index + 3] = c | 0b1'0000000;
        buffer[buffer_index + 4] = (char)output_char_index +1 | 0b1'0000000;
        buffer[buffer_index + 5] = c | 0b1'0000000;
        buffer[buffer_index + 6] = (char)output_char_index +1 | 0b1'0000000;
        buffer[buffer_index + 7] = c | 0b0'0000000;


        buffer_index += 8;
    skip:
        output_char_index += 1;
        if (buffer_index == 512 || output_char_index == 128) break;
    }


    LPDIRECT3DVERTEXBUFFER8 result_vb = NULL;
    if (FAILED(Graphics_GetD3D()->CreateVertexBuffer(buffer_index, 0, 0, 0, &result_vb))) {
        return 0;
    }

    void* vert_buffer;
    if (FAILED(result_vb->Lock(0, 0, (BYTE**)&vert_buffer, 0))) {
        result_vb->Release();
        return 0;
    }
    memcpy(vert_buffer, buffer, buffer_index);
    result_vb->Unlock();

    *output_vertex_count = buffer_index / 8;
    return result_vb;
}

void Log(const char* string, const float* color, bool display_extra_as_int, void* extra_to_display) {
    if (!Graphics_GetD3D()) return; // no logs if we arent init

    // release oldest log if buffer is full, as it'll be overwritten
    if (active_log_count >= max_logs) {
        logs[15].vertex_buffer_handle->Release();
    }
    
    // shift logs up to free up index 0
    int i = active_log_count;
    if (i > 15) i = 15;
    i -= 1;

    while (i >= 0) {
        memcpy(&logs[i+1], &logs[i], sizeof(log_entry));
        i -= 1;
    }

    // convert string to vertex buffer and write to submit to log
    logs[0].vertex_buffer_handle = ConstructStringBuffer(string, &logs[0].vertex_count, display_extra_as_int, extra_to_display);
    memcpy(logs[0].color, color, 12);

    if (active_log_count < max_logs) active_log_count += 1;
}


void Log_Render() {
	// process queue into vertex buffers
    auto d3d = Graphics_GetD3D();

    // apply global constants
    float c0[4] = { 4.0f, 5.0f, 4.0f / 32.0f, 5.0f / 32.0f };
    float c1[4] = { 8.0f, 1.0f, 128.0f, 255.0f };
    float c2[4] = {
        0.0f, // must be zero
        2.0f / 640.0f, // screenScaleX
        2.0f / 480.0f, // screenScaleY
        6.0f, // glyphWidthPixelsSpacer
    };
    d3d->SetVertexShaderConstant(0, c0, 1);
    d3d->SetVertexShaderConstant(1, c1, 1);
    d3d->SetVertexShaderConstant(2, c2, 1);

    // loop through all logs and render
    for (int line_index = 0; line_index < active_log_count; line_index++) {
        // vert buffer must be valid
        if (!logs[line_index].vertex_buffer_handle)
            continue;
        

        d3d->SetStreamSource(0, logs[line_index].vertex_buffer_handle, 2);

        // adjustable text display params
        float c3[4] = {
            logs[line_index].color[0],
            logs[line_index].color[1],
            logs[line_index].color[2],
            10.0f * line_index + 5.0f
        };
        d3d->SetVertexShaderConstant(3, c3, 1);
        d3d->DrawPrimitive(D3DPT_QUADLIST, 0, logs[line_index].vertex_count);
    }

}
