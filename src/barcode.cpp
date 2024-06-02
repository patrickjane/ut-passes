// **************************************************************************
// class BarcodeGenerator
// 02.07.2021
// Generation of barcode images using the zxing library
// **************************************************************************
// MIT License
// Copyright © 2021 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#include <QDebug>

#include "barcode.h"

#include "ZXing/MultiFormatWriter.h"
#include "ZXing/BarcodeFormat.h"
#include "ZXing/BitMatrix.h"
#include "ZXing/ByteMatrix.h"
#include "ZXing/MultiFormatWriter.h"
#include "ZXing/TextUtfEncoding.h"
#include "ZXing/CharacterSetECI.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb_image_write.h"

namespace C {
#include <libintl.h>
}

namespace passes
{
   // **************************************************************************
   // class BarcodeGenerator
   // **************************************************************************

   QString BarcodeGenerator::generate(QString text, QString fmt, QImage* dest)
   {
      using namespace ZXing;

      int width = 500, height = 500;
      int margin = 5;
      int eccLevel = -1;
      CharacterSet encoding = CharacterSet::UTF8;
      BarcodeFormat format;

      if (fmt == "PKBarcodeFormatPDF417")
         format = BarcodeFormat::PDF_417;
      else if (fmt == "PKBarcodeFormatAztec")
         format = BarcodeFormat::AZTEC;
      else if (fmt == "PKBarcodeFormatQR")
         format = BarcodeFormat::QR_CODE;
      else if (fmt == "PKBarcodeFormatCode128")
         format = BarcodeFormat::CODE_128;
      else if (fmt == "CODE_39")
         format = BarcodeFormat::CODE_39;
      else if (fmt == "EAN-8")
         format = BarcodeFormat::EAN_8;
      else if (fmt == "EAN-13")
         format = BarcodeFormat::EAN_13;
      else if (fmt == "UPC-A")
         format = BarcodeFormat::UPC_A;
      else
         return QString(C::gettext("Unknown barcode format")) + " (" + fmt + ")";

      MultiFormatWriter writer(format);
      if (margin >= 0)
         writer.setMargin(margin);
      if (eccLevel >= 0)
         writer.setEccLevel(eccLevel);

      auto bitmap = writer.encode(TextUtfEncoding::FromUtf8(text.toUtf8().constData()), width, height).toByteMatrix();

      QByteArray dataArray;

      int res = stbi_write_png_to_func(&BarcodeGenerator::stbiWriteFunc, &dataArray, bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);

      dest->loadFromData(dataArray);
      return "";
   }

   void BarcodeGenerator::stbiWriteFunc(void* context, void* data, int size)
   {
      QByteArray* dataArray = (QByteArray*)context;

      if (!dataArray)
         return;

      dataArray->append((const char*)data, size);
   }
}
