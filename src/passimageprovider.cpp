// **************************************************************************
// class PassImageProvider
// 02.07.2021
// Model for handling locally stored passes managed by this app
// **************************************************************************
// MIT License
// Copyright © 2021 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of the Software. THE
// SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#include "passimageprovider.h"
#include "passesmodel.h"

#include <QDebug>

// **************************************************************************
// class PassImageProvider
// **************************************************************************

namespace passes {
QImage PassImageProvider::requestImage(const QString& id, QSize* /*size*/,
                                       const QSize& /*requestedSize*/)
{
    PassesModel* model = PassesModel::getInstace();

    if (!model)
        return QImage();

    QStringList comps = id.split("/");

    if (comps.size() < 2)
        return QImage();

    PassPtr pass = model->getPass(comps[0]);

    if (!pass)
        return QImage();

    if (comps[1] == "background")
        return pass->imgBackground;
    if (comps[1] == "footer")
        return pass->imgFooter;
    if (comps[1] == "icon")
        return pass->imgIcon;
    if (comps[1] == "logo")
        return pass->imgLogo;
    if (comps[1] == "strip")
        return pass->imgStrip;
    if (comps[1] == "thumbnail")
        return pass->imgThumbnail;

    if (comps[1] == "barcode") {
        int index = 0;
        int bundleIndex = -1;

        if (comps.size() >= 3)
            index = comps[2].toInt();
        if (comps.size() >= 4)
            bundleIndex = comps[3].toInt();

        if (bundleIndex > -1 && bundleIndex < pass->bundlePasses.size())
            pass = pass->bundlePasses[bundleIndex];

        if (index < 0 || index >= pass->standard.barcodes.size())
            return QImage();

        return pass->standard.barcodes[index].image;
    }

    return QImage();
}

} // namespace passes
