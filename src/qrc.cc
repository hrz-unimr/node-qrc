#include <cstdlib>
#include <cstring>
#include <string>

#include <errno.h>

#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include <qrencode.h>
#include <png.h>

using namespace v8;

// for compatibility with libqrencode < 3.2.0:
#ifndef QRSPEC_VERSION_MAX
#define QRSPEC_VERSION_MAX 40
#endif

const unsigned int QRC_MAX_SIZE[] = { 2938, 2319, 1655, 1268 };
const int WHITE = 16777216;

struct Qrc_Params {
	unsigned char *data;
	QRecLevel ec_level;
	QRencodeMode mode;
	int dot_size;
	int margin;
	int foreground_color;
	int background_color;
	int version;

	Qrc_Params(std::string p_data, QRecLevel p_ec_level = QR_ECLEVEL_L, QRencodeMode p_mode = QR_MODE_8,
			int p_version = 0,
			int p_dot_size = 3, int p_margin = 4,
			int p_foreground_color = 0x0, int p_background_color = 0xffffff) {
		data = new unsigned char[p_data.length() + 1];
		std::strncpy((char *)data, p_data.c_str(), p_data.length() + 1);
		ec_level = p_ec_level;
		mode = p_mode;
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
		free(data);
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
			delete params;
			ThrowException(Exception::TypeError(String::New("Second argument must be an object")));
			return NULL;
		}
		Local<Object> paramsObj = Local<Object>::Cast(args[1]);
		Local<Value> paramsVersion = paramsObj->Get(String::New("version"));
		if (!paramsVersion->IsUndefined()) {
			if (!paramsVersion->IsInt32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for version")));
				return NULL;
			} else if (paramsVersion->IntegerValue() < 1 || paramsVersion->IntegerValue() > QRSPEC_VERSION_MAX) {
				delete params;
				ThrowException(Exception::RangeError(String::New("Version out of range")));
				return NULL;
			} else {
				params->version = paramsVersion->IntegerValue();
			}
		}
		Local<Value> paramsEcLevel = paramsObj->Get(String::New("ecLevel"));
		if (!paramsEcLevel->IsUndefined()) {
			if (!paramsEcLevel->IsInt32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for EC level")));
				return NULL;
			} else if (paramsEcLevel->IntegerValue() < QR_ECLEVEL_L || paramsEcLevel->IntegerValue() > QR_ECLEVEL_H) {
				delete params;
				ThrowException(Exception::RangeError(String::New("EC level out of range")));
				return NULL;
			} else {
				params->ec_level = (QRecLevel) paramsEcLevel->IntegerValue();
				if (data.length() > QRC_MAX_SIZE[params->ec_level]) {
					delete params;
					ThrowException(Exception::RangeError(String::New("Source string length out of range")));
					return NULL;
				}
			}
		}
		Local<Value> paramsMode = paramsObj->Get(String::New("mode"));
		if (!paramsMode->IsUndefined()) {
			if (!paramsMode->IsInt32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for mode")));
				return NULL;
			} else if (paramsMode->IntegerValue() < QR_MODE_NUM || paramsMode->IntegerValue() > QR_MODE_KANJI) {
				delete params;
				ThrowException(Exception::RangeError(String::New("Mode out of range")));
				return NULL;
			} else {
				params->mode = (QRencodeMode) paramsMode->IntegerValue();
				// TODO check length of data
			}
		}
		Local<Value> paramsDotSize = paramsObj->Get(String::New("dotSize"));
		if (!paramsDotSize->IsUndefined()) {
			if (!paramsDotSize->IsInt32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for dot size")));
				return NULL;
			} else if (paramsDotSize->IntegerValue() < 1 || paramsDotSize->IntegerValue() > 50) {
				delete params;
				ThrowException(Exception::RangeError(String::New("Dot size out of range")));
				return NULL;
			} else {
				params->dot_size = paramsDotSize->IntegerValue();
			}
		}
		Local<Value> paramsMargin = paramsObj->Get(String::New("margin"));
		if (!paramsMargin->IsUndefined()) {
			if (!paramsMargin->IsInt32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for margin size")));
				return NULL;
			} else if (paramsMargin->IntegerValue() < 0 || paramsMargin->IntegerValue() > 10) {
				delete params;
				ThrowException(Exception::RangeError(String::New("Margin size out of range")));
				return NULL;
			} else {
				params->margin = paramsMargin->IntegerValue();
			}
		}
		Local<Value> paramsFgColor = paramsObj->Get(String::New("foregroundColor"));
		if (!paramsFgColor->IsUndefined()) {
			if (!paramsFgColor->IsUint32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for foreground color")));
				return NULL;
			} else if (paramsFgColor->IntegerValue() < 0 || paramsFgColor->IntegerValue() >= WHITE) {
				delete params;
				ThrowException(Exception::RangeError(String::New("Foreground color out of range")));
				return NULL;
			} else {
				params->foreground_color = paramsFgColor->IntegerValue();
			}
		}
		Local<Value> paramsBgColor = paramsObj->Get(String::New("backgroundColor"));
		if (!paramsBgColor->IsUndefined()) {
			if (!paramsBgColor->IsUint32()) {
				delete params;
				ThrowException(Exception::TypeError(String::New("Wrong type for background color")));
				return NULL;
			} else if (paramsBgColor->IntegerValue() < 0 || paramsBgColor->IntegerValue() >= WHITE) {
				delete params;
				ThrowException(Exception::RangeError(String::New("Background color out of range")));
				return NULL;
			} else {
				params->background_color = paramsBgColor->IntegerValue();
			}
		}
	}

	return params;
}


QRcode* Encode(Qrc_Params* params) {
	QRinput *input;
	if ((input = QRinput_new2(params->version, params->ec_level)) == NULL) {
		if (errno == EINVAL) {
			ThrowException(Exception::Error(String::New("Input data is invalid")));
			return NULL;
		}
		if (errno == ENOMEM) {
			ThrowException(Exception::Error(String::New("Not enough memory")));
			return NULL;
		}
	}
	if (QRinput_append(input, params->mode, strlen((const char *)params->data), params->data) == -1) {
		if (errno == EINVAL) {
			ThrowException(Exception::Error(String::New("Input data is invalid")));
			return NULL;
		}
		if (errno == ENOMEM) {
			ThrowException(Exception::Error(String::New("Not enough memory")));
			return NULL;
		}
	}
	QRcode *code = QRcode_encodeInput(input);
	QRinput_free(input);

	if (code == NULL) {
		ThrowException(Exception::Error(String::New("Could not encode input")));
		return NULL;
	}

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
	if (code) {
		Local<node::Buffer> buffer = node::Buffer::New((const char*)code->data, code->width * code->width);
		codeObj->Set(String::NewSymbol("width"), Integer::New(code->width));
		codeObj->Set(String::NewSymbol("version"), Integer::New(code->version));
		codeObj->Set(String::NewSymbol("data"), buffer->handle_);
		QRcode_free(code);
	}
	return scope.Close(codeObj);
}


void Qrc_png_write_buffer(png_structp png_ptr, png_bytep data, png_size_t length) {
	Qrc_Png_Buffer *b = (Qrc_Png_Buffer *)png_get_io_ptr(png_ptr);
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

	QRcode *code = Encode(params);

	if (code) {
		Qrc_Png_Buffer* bp = new Qrc_Png_Buffer();

		png_structp png_ptr;
		png_infop info_ptr;
		png_colorp png_plte;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
		if (!png_ptr) {
			delete params;
			QRcode_free(code);
			return scope.Close(obj);
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			delete params;
			QRcode_free(code);
			return scope.Close(obj);
		}

		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_write_struct(&png_ptr, &info_ptr);
			delete params;
			QRcode_free(code);
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
