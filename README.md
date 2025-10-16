#  VM Builder — автоматическое создание и запуск виртуальной машины с cloud-init

## Требования

Перед использованием убедитесь, что установлены следующие пакеты:

```bash
sudo apt install qemu-kvm libvirt-daemon-system virtinst genisoimage -y
```
И доступен  образ Ubuntu cloud-init `jammy-server-cloudimg-amd64.img` в папке со скриптом

## Использование
Соберите программу с помощью GCC:

```bash
gcc vm_builder.c -o vm_builder
```
Запустите программу:
```bash
./vm_builder <vm_name> <ram_mb> <vcpus> "<web_message>" [<virt_type>]
```

**Параметры:**

-   `<vm_name>` — имя создаваемой виртуальной машины
-   `<ram_mb>` — объём оперативной памяти (в мегабайтах)
-   `<vcpus>` — количество виртуальных процессоров
-   `<web_message>` — текст, который появится на странице nginx внутри ВМ
-   `<virt_type>` — тип гипервизора (`kvm` или `qemu`). Если не указывать, то используется `qemu`

После запуска программы создаются `user-data`, `meta-data`, `cloud-data.iso` и запускается создание ВМ. В самом конце выводится адрес, по которому доступен nginx.
