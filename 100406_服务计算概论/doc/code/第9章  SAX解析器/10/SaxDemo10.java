import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo10 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("10.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    boolean flag = false;
    Locator locator;
    int line,column;
    public void setDocumentLocator(Locator locator) {
        this.locator=locator;
    }

    public void startDocument() throws SAXException {
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
        if(qName.equals("element")){
            flag = true;
        }
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        if(flag){
        System.out.print(new String(ch,start,length));
            line = locator.getLineNumber();
            column = locator.getColumnNumber();
            System.out.println("(所在位置：第"+line+"行，第"+column+"列)");
        flag = false;
        }
    }
}

