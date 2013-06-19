#include <cstdlib>
#include <string>
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <qrencode.h>
#include <png.h>

using namespace v8;

const unsigned int QRC_MAX_SIZE[] = { 2938, 2319, 1655, 1268 };
const int WHITE = 16777216;

struct Qrc_Params {
  const char* data;
  QRecLevel ec_level;
  int dot_size;
  int margin;
  int foreground_color;
  int background_color;
  int version;

  Qrc_Params(std::string p_data, QRecLevel p_ec_level = QR_ECLEVEL_L,
          int p_version = 0,
          int p_dot_size = 3, int p_margin = 4,
          int p_foreground_color = 0x0, int p_background_color = 0xffffff) {
    data = new char[p_data.length() + 1];
    strcpy((char*)data, p_data.c_str());
    ec_level = p_ec_level;
    version = p_version;
    dot_size = p_dot_size;
    margin = p_margin;
    foreground_color = p_foreground_color;
    background_color = p_background_color;
  }

  ~Qrc_Params() {
    delete data;
  }
};

struct Qrc_Png_Buffer {
  char *data;
  size_t size;
  Qrc_Png_Buffer() {
    data = NULL;
    size = 0;
  }
  ~Qrc_Png_Buffer() {
    if (data) {
      free(data);
    }
  }
};

Qrc_Params* ValidateArgs(const Arguments& args) {
  struct Qrc_Params* params;

  if (args.Length() < 1 || !args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("No source string given")));
    return NULL;
  }
  std::string data(*v8::String::Utf8Value(args[0]));
  if (data.length() < 1 || data.length() > QRC_MAX_SIZE[0]) {
    ThrowException(Exception::RangeError(String::New("Source string length out of range")));
    return NULL;
  }
  params = new Qrc_Params(data);

  if (args.Length() > 1) {
    if (!args[1]->IsObject()) {
      ThrowException(Exception::TypeError(String::New("Second argument must be an object")));
      delete params;
      return NULL;
    }
    Local<Object> paramsObj = Local<Object>::Cast(args[1]);
    Local<Value> paramsVersion = paramsObj->Get(String::New("version"));
    if (!paramsVersion->IsUndefined()) {
      if (!paramsVersion->IsInt32()) {
        ThrowException(Exception::TypeError(String::New("Wrong type for version")));
        delete params;
        return NULL;
      } else if (paramsVersion->IntegerValue() < 1 || paramsVersion->IntegerValue() > QRSPEC_VERSION_MAX) {
        ThrowException(Exception::RangeError(String::New("Version out of range")));
        delete params;
        return NULL;
      } else {
        params->version = paramsVersion->IntegerValue();
      }
    }
    Local<Value> paramsEcLevel = paramsObj->Get(String::New("ecLevel"));
    if (!paramsEcLevel->IsUndefined()) {
      if (!paramsEcLevel->IsInt32()) {
        ThrowException(Exception::TypeError(String::New("Wrong type for EC level")));
        delete params;
        return NULL;
      } else if (paramsEcLevel->IntegerValue() < QR_ECLEVEL_L || paramsEcLevel->IntegerValue() > QR_ECLEVEL_H) {
        ThrowException(Exception::RangeError(String::New("EC level out of range")));
        delete params;
        return NULL;
      } else {
        params->ec_level = (QRecLevel) paramsEcLevel->IntegerValue();
        if (data.length() > QRC_MAX_SIZE[params->ec_level]) {
          ThrowException(Exception::RangeError(String::New("Source string length out of range")));
          delete params;
          return NULL;
        }
      }
    }
    Local<Value> paramsDotSize = paramsObj->Get(String::New("dotSize"));
    if (!paramsDotSize->IsUndefined()) {
      if (!paramsDotSize->IsInt32()) {
        ThrowException(Exception::TypeError(String::New("Wrong type for dot size")));
        delete params;
        return NULL;
      } else if (paramsDotSize->IntegerValue() < 1 || paramsDotSize->IntegerValue() > 50) {
        ThrowException(Exception::RangeError(String::New("Dot size out of range")));
        delete params;
        return NULL;
      } else {
        params->dot_size = paramsDotSize->IntegerValue();
      }
    }
    Local<Value> paramsMargin = paramsObj->Get(String::New("margin"));
    if (!paramsMargin->IsUndefined()) {
      if (!paramsMargin->IsInt32()) {
        ThrowException(Exception::TypeError(String::New("Wrong type for margin size")));
        delete params;
        return NULL;
      } else if (paramsMargin->IntegerValue() < 0 || paramsMargin->IntegerValue() > 10) {
        ThrowException(Exception::RangeError(String::New("Margin size out of range")));
        delete params;
        return NULL;
      } else {
        params->margin = paramsMargin->IntegerValue();
      }
    }
    Local<Value> paramsFgColor = paramsObj->Get(String::New("foregroundColor"));
    if (!paramsFgColor->IsUndefined()) {
      if (!paramsFgColor->IsUint32()) {
        ThrowException(Exception::TypeError(String::New("Wrong type for foreground color")));
        delete params;
        return NULL;
      } else if (paramsFgColor->IntegerValue() < 0 || paramsFgColor->IntegerValue() >= WHITE) {
        ThrowException(Exception::RangeError(String::New("Foreground color out of range")));
        delete params;
        return NULL;
      } else {
        params->foreground_color = paramsFgColor->IntegerValue();
      }
    }
    Local<Value> paramsBgColor = paramsObj->Get(String::New("backgroundColor"));
    if (!paramsBgColor->IsUndefined()) {
      if (!paramsBgColor->IsUint32()) {
        ThrowException(Exception::TypeError(String::New("Wrong type for background color")));
        delete params;
        return NULL;
      } else if (paramsBgColor->IntegerValue() < 0 || paramsBgColor->IntegerValue() >= WHITE) {
        ThrowException(Exception::RangeError(String::New("Background color out of range")));
        delete params;
        return NULL;
      } else {
        params->background_color = paramsBgColor->IntegerValue();
      }
    }
  }

  return params;
}


QRcode* Encode(Qrc_Params* params) {
  QRcode* code;
  code = QRcode_encodeString8bit((const char*)params->data, params->version, params->ec_level);
  return code;
}


Handle<Value> EncodeBuf(const Arguments& args) {
  HandleScope scope;
  Local<Object> codeObj = Object::New();

  Qrc_Params* params = ValidateArgs(args);
  if (!params) {
    return scope.Close(codeObj);
  }

  QRcode* code = Encode(params);
  delete params;
  if (code != NULL) {
    Local<node::Buffer> buffer = node::Buffer::New((const char*)code->data, code->width * code->width);
    codeObj->Set(String::NewSymbol("width"), Integer::New(code->width));
    codeObj->Set(String::NewSymbol("version"), Integer::New(code->version));
    codeObj->Set(String::NewSymbol("data"), buffer->handle_);
    QRcode_free(code);
  }
  return scope.Close(codeObj);
}


void Qrc_png_write_buffer(png_structp png_ptr, png_bytep data, png_size_t length) {
  Qrc_Png_Buffer* b = (Qrc_Png_Buffer *)png_get_io_ptr(png_ptr);
  size_t nsize = b->size + length;

  if (b->data) {
    b->data = (char *)realloc(b->data, nsize);
  } else {
    b->data = (char *)malloc(nsize);
  }

  if (!b->data) {
    png_error(png_ptr, "Write Error");
  }

  memcpy(b->data + b->size, data, length);
  b->size += length;
}


Handle<Value> EncodePNG(const Arguments& args) {
  HandleScope scope;
  Local<Object> obj = Object::New();

  Qrc_Params* params = ValidateArgs(args);
  if (!params) {
    return scope.Close(obj);
  }

  QRcode* code = Encode(params);

  if (code != NULL) {
    Qrc_Png_Buffer* bp = new Qrc_Png_Buffer();

    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp png_plte;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
        NULL, NULL, NULL);
    if (!png_ptr) {
      delete params;
      return scope.Close(obj);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      delete params;
      return scope.Close(obj);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      delete params;
      return scope.Close(obj);
    }

    png_set_write_fn(png_ptr, bp, Qrc_png_write_buffer, NULL);

    png_plte = (png_colorp) malloc(sizeof(png_color) * 2);
    png_plte[0].red = params->background_color >> 16 & 0xFF;
    png_plte[0].green = params->background_color >> 8 & 0xFF;
    png_plte[0].blue = params->background_color & 0xFF;
    png_plte[1].red = params->foreground_color >> 16 & 0xFF;
    png_plte[1].green = params->foreground_color >> 8 & 0xFF;
    png_plte[1].blue = params->foreground_color & 0xFF;

    png_set_PLTE(png_ptr, info_ptr, png_plte, 2);

    png_set_IHDR(png_ptr, info_ptr, (code->width + params->margin * 2) * params->dot_size, (code->width + params->margin * 2) * params->dot_size, 1,
        PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_set_packing(png_ptr);

    unsigned char* row = new unsigned char[(code->width + params->margin * 2) * params->dot_size];

    for (int y = -(params->margin); y < code->width + params->margin; y++) {
      for (int x = -(params->margin * params->dot_size); x < (code->width + params->margin) * params->dot_size; x += params->dot_size) {
        for (int d = 0; d < params->dot_size; d++) {
          if (y < 0 || y > code->width - 1 || x < 0 || x > ((code->width - 1) * params->dot_size)) {
            row[x + params->margin * params->dot_size + d] = 0;
          } else {
            row[x + params->margin * params->dot_size + d] = code->data[y * code->width + x/params->dot_size] << 7;
          }
        }
      }
      for (int d = 0; d < params->dot_size; d++) {
        png_write_row(png_ptr, row);
      }
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    delete[] row;
    free(png_plte);

    Local<node::Buffer> buffer = node::Buffer::New(bp->data, bp->size);
    obj->Set(String::NewSymbol("width"), Integer::New(code->width));
    obj->Set(String::NewSymbol("version"), Integer::New(code->version));
    obj->Set(String::NewSymbol("data"), buffer->handle_);
    QRcode_free(code);
    delete bp;
  }
  delete params;
  return scope.Close(obj);
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("encode"),
      FunctionTemplate::New(EncodeBuf)->GetFunction());
  exports->Set(String::NewSymbol("encodePng"),
      FunctionTemplate::New(EncodePNG)->GetFunction());
}

NODE_MODULE(qrc, init)
