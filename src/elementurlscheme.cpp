#include "elementurlscheme.hpp"

#include <QWebEngineUrlRequestJob>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

ElementUrlScheme::ElementUrlScheme(const QString &root, QObject *parent)
    : QWebEngineUrlSchemeHandler(parent),
      root(root)
{
}

void ElementUrlScheme::requestStarted(QWebEngineUrlRequestJob *request)
{
    // check if directory exists with every request in case it is moved or deleted
    if (!QFileInfo(root).isDir())
    {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }

    // normalize path
    const auto path = ElementUrlScheme::getFilePath(request->requestUrl());
    const auto fullPath = QString("%1/%2").arg(root, path);

    // check if file exists
    if (!QFileInfo::exists(fullPath))
    {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
    }

    // prepare file for reading
    auto file = new QFile(fullPath, this);
    connect(request, &QObject::destroyed, file, &QObject::deleteLater);

    if (!file->open(QIODevice::ReadOnly))
    {
        // permission denied reading file, respond with request denied
        request->fail(QWebEngineUrlRequestJob::RequestDenied);
    }

    // send file
    request->reply(ElementUrlScheme::mimeType(fullPath), file);
}

const QString ElementUrlScheme::getFilePath(const QUrl &url)
{
    // get requested path
    const auto path = url.path(/*QUrl::FullyEncoded*/);

    // return index.html
    if (path.isEmpty() || path == '/')
    {
        return "index.html";
    }

    // return requested file
    else
    {
        return path.mid(1);
    }
}

const QByteArray ElementUrlScheme::mimeType(const QString &path)
{
    const QMimeDatabase db;
    return db.mimeTypeForFile(path).name().toUtf8();
}
