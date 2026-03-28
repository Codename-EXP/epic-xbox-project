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


LPDIRECT3DVERTEXBUFFER8 ConstructStringBuffer(const char* string, UINT* output_vertex_count) {
    if (!string) return 0;

    char buffer[512];
    UINT string_index = 0; // breaks at null terminator
    UINT buffer_index = 0; // auto breaks at index 512

    while (char c = string[string_index]) {

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

    submit:
        // create for vertices
        buffer[buffer_index]     = (char)string_index+1 | 0b0'0000000;
        buffer[buffer_index + 1] = c | 0b0'0000000;
        buffer[buffer_index + 2] = (char)string_index+1 | 0b0'0000000;
        buffer[buffer_index + 3] = c | 0b1'0000000;
        buffer[buffer_index + 4] = (char)string_index+1 | 0b1'0000000;
        buffer[buffer_index + 5] = c | 0b1'0000000;
        buffer[buffer_index + 6] = (char)string_index+1 | 0b1'0000000;
        buffer[buffer_index + 7] = c | 0b0'0000000;


        buffer_index += 8;
    skip:
        string_index += 1;
        if (buffer_index == 512 || string_index == 128) break;
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

void Log(const char* string, const float* color) {
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
    logs[0].vertex_buffer_handle = ConstructStringBuffer(string, &logs[0].vertex_count);
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
