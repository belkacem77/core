/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/DocumentEncryption.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>

namespace oox {
namespace core {

using namespace css::io;
using namespace css::uno;

DocumentEncryption::DocumentEncryption(Reference<XStream> const & xDocumentStream,
                                       oox::ole::OleStorage& rOleStorage,
                                       const OUString& rPassword)
    : mxDocumentStream(xDocumentStream)
    , mrOleStorage(rOleStorage)
    , maPassword(rPassword)
{}

bool DocumentEncryption::encrypt()
{
    Reference<XInputStream> xInputStream (mxDocumentStream->getInputStream(), UNO_SET_THROW);
    Reference<XSeekable> xSeekable(xInputStream, UNO_QUERY);

    if (!xSeekable.is())
        return false;

    sal_uInt32 aLength = xSeekable->getLength(); // check length of the stream
    xSeekable->seek(0); // seek to begin of the document stream

    if (!mrOleStorage.isStorage())
        return false;

    mEngine.setupEncryption(maPassword);

    Reference<XOutputStream> xOutputStream(mrOleStorage.openOutputStream("EncryptedPackage"), UNO_SET_THROW);

    mEngine.encrypt(xInputStream, xOutputStream, aLength);

    xOutputStream->flush();
    xOutputStream->closeOutput();

    Reference<XOutputStream> xEncryptionInfo(mrOleStorage.openOutputStream("EncryptionInfo"), UNO_SET_THROW);
    BinaryXOutputStream aEncryptionInfoBinaryOutputStream(xEncryptionInfo, false);

    mEngine.writeEncryptionInfo(aEncryptionInfoBinaryOutputStream);

    aEncryptionInfoBinaryOutputStream.close();
    xEncryptionInfo->flush();
    xEncryptionInfo->closeOutput();

    return true;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
