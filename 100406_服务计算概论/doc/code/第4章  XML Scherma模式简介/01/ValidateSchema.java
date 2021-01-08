import org.w3c.dom.Document;
import org.w3c.dom.DOMException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;


public class ValidateSchema {
    public static void main(String[] args) {
        String  xmlfile="score.xml";
        String  xsdfile="score.xsd";
        MyHandler mh = null;
        try {
            DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory.newInstance();
            docBuilderFactory.setNamespaceAware(true);
            docBuilderFactory.setValidating(true);
            docBuilderFactory.setAttribute("http://java.sun.com/xml/jaxp/properties/schemaLanguage", "http://www.w3.org/2001/XMLSchema");
            docBuilderFactory.setAttribute("http://java.sun.com/xml/jaxp/properties/schemaSource", xsdfile);
            DocumentBuilder docBuilder = docBuilderFactory.newDocumentBuilder();
            mh = new MyHandler();
            docBuilder.setErrorHandler(mh);
            Document doc = docBuilder.parse(xmlfile);
            if(mh.em==null){
                System.out.println("�ļ�"+xmlfile+"����"+xsdfile+"�ļ�ģʽ����Ч��XML�ļ���");
            }else{
                System.out.println("�ļ�"+xmlfile+"������"+xsdfile+"�ļ�ģ����Ч��XML�ļ���");
            }
        } catch(Exception e){
            System.out.println(e);
        }

    }

}
