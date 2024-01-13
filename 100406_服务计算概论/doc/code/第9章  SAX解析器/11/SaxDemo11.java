import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo11 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            factory.setValidating(true);
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("11.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    boolean flag = false;
    int line,column;
    public void startDocument() throws SAXException {
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.println("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
       System.out.println(new String(ch,start,length));
    }

    public void warning(SAXParseException e) throws SAXException {
        line = e.getLineNumber();
        column = e.getColumnNumber();
        System.out.println("警告："+e.getMessage()+"位置：行"+line+"列"+column);
    }

    public void error(SAXParseException e) throws SAXException {
        line = e.getLineNumber();
        column = e.getColumnNumber();
        System.out.println("一般错误："+e.getMessage()+"位置：行"+line+"列"+column);
    }

    public void fatalError(SAXParseException e) throws SAXException {
        line = e.getLineNumber();
        column = e.getColumnNumber();
        System.out.println("严重错误："+e.getMessage()+"位置：行"+line+"列"+column);
        throw new SAXException("文件中发现致命错误，解析无法完成");
    }
}

