import org.w3c.dom.*;
import javax.xml.parsers.*;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.File;

public class Add {
    public static void main(String[] args) {
        boolean flag ;
        Document document = null;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(new File("02.xml"));
            document.normalize();
            Node root = document.getDocumentElement();
            if (root.hasChildNodes()) {

                /* ����student�ڵ� */
                Element selement = document.createElement("student");
                root.appendChild(selement);

                /*Ϊstudent ����ӽڵ� */
                Element element = document.createElement("name");
                element.appendChild(document.createTextNode("����"));
                selement.appendChild(element);

                element = document.createElement("sex");
                element.appendChild(document.createTextNode("��"));
                selement.appendChild(element);

                element = document.createElement("age");
                element.appendChild(document.createTextNode("24"));
                selement.appendChild(element);

                /* ����Document */
               flag = SaveXmlFile(document, "02.xml");
               if(flag){
                   System.out.println("�ɹ��������ݣ�");
               }else{
                   System.out.println("����ʧ�ܣ�");
               }
            }
        }catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public static boolean SaveXmlFile(Document document, String filename) {
        boolean flag = true;
        try {
            /** ��document�е�����д���ļ���  */
            TransformerFactory tFactory = TransformerFactory.newInstance();
            Transformer transformer = tFactory.newTransformer();
            /** ���� */
            //transformer.setOutputProperty(OutputKeys.ENCODING, "GB2312");
            DOMSource source = new DOMSource(document);
            StreamResult result = new StreamResult(new File(filename));
            transformer.transform(source, result);
        } catch (Exception ex) {
            flag = false;
            ex.printStackTrace();
        }
        return flag;
    }
}
