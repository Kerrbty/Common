#include <iostream>
#include <fstream>
#include <conio.h>
#include "person.pb.h"

#ifdef _DEBUG
#pragma comment(lib, "libprotobuf_d")
#else
#pragma comment(lib, "libprotobuf")
#endif

int main(int argc, char* argv[])
{
    Person person;
    person.set_id(8888);
    person.set_name("Kobe");
    person.set_email("1990@qq.com");

    //������д��person.pb�ļ�
    std::fstream out("person.xml", std::ios::out | std::ios::binary | std::ios::trunc);
    person.SerializeToOstream(&out);
    out.close();

    //��person.pb�ļ���ȡ����
    std::fstream in("person.xml", std::ios::in | std::ios::binary);
    if (!person.ParseFromIstream(&in)) {
        std::cerr << "Failed to parse person.xml." << std::endl;
        exit(1);
    }

    std::cout << "ID: " << person.id() << std::endl;
    std::cout << "name: " << person.name() << std::endl;
    if (person.has_email()) {
        std::cout << "e-mail: " << person.email() << std::endl;
    }

    getch();
    return 0;
}
