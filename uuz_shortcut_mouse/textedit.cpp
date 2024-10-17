#include "textedit.h"


TextEdit::TextEdit(QWidget* parent) : QTextEdit(parent) {
  connect(this, &TextEdit::sig_editModify, Search_content::get_searchContentWidget(),
          &Search_content::slot_configFilterModify);
}

TextEdit::~TextEdit() {}


void TextEdit::focusOutEvent(QFocusEvent* event) {
#ifdef _DEBUG
	qDebug() << "TextEdit失去焦点";
#endif

  QStringList* string_list = nullptr; //引用必须初始化，只能用指针间接一下了
  if (this->objectName() == "textEdit_filterPath") {
    string_list = &Search_content::get_filter().first;
  }
  else if (this->objectName() == "textEdit_filterSuffix") {
    string_list = &Search_content::get_filter().second;
  }

  content = this->toPlainText().split("\n");

  if (content == *string_list) {
#ifdef _DEBUG
		qDebug() << "文本相同不进行修改";
#endif
    return;
  }

  *string_list = content;

#ifdef _DEBUG
	for (auto& str : content) {
		qDebug() << str;
	}
#endif

  emit sig_editModify();
  QTextEdit::focusOutEvent(event);
}
