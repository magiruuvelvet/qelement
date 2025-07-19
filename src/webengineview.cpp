#include "webengineview.hpp"

#include <QDesktopServices>

WebEngineView::WebEngineView(QWebEngineProfile *profile, QWidget *parent)
    : QWebEngineView(parent)
{
    this->profile = profile;
    this->_page = std::make_unique<WebEnginePage>(profile, this);
    this->setPage(this->_page.get());
}

void WebEngineView::setPage(WebEnginePage *page)
{
    QWebEngineView::setPage(page);
}

WebEnginePage *WebEngineView::page() const
{
    return this->_page.get();
}

WebEngineView *WebEngineView::createWindow(QWebEnginePage::WebWindowType type)
{
    return nullptr;
}
