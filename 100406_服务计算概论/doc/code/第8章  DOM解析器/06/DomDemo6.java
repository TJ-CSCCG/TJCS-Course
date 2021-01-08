import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo6 {
     public static void main(String[] args) {
         int m=1;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("05.xml"));

            DocumentType docType = document.getDoctype();
            System.out.println("��������ƣ�"+docType.getName());
            System.out.println("��ʽ���ñ�ʶ����"+docType.getPublicId());
            System.out.println("�������ⲿDTD��"+docType.getSystemId());
            System.out.println("�ļ��ڲ���DTD��\n"+docType.getInternalSubset());
            } catch (Exception e) {
            e.printStackTrace();
        }

    }



}
