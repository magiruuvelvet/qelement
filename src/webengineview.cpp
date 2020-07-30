#include "webengineview.hpp"

#include <QDesktopServices>

WebEngineView::WebEngineView(QWidget *parent)
    : QWebEngineView(parent)
{
    this->_page = std::make_unique<WebEnginePage>(this);
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
