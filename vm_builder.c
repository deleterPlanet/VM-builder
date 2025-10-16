#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_BUFFER 1024

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <vm_name> <ram_mb> <vcpus> <web_message>\n", argv[0]);
        return 1;
    }

    char *vm_name = argv[1];
    char *ram = argv[2];
    char *vcpus = argv[3];
    char *message = argv[4];
    char cmd[CMD_BUFFER];

    printf("Create user-data\n");
    FILE *f = fopen("user-data", "w");
    fprintf(f,
        "#cloud-config\n"
        "hostname: %s\n"
        "manage_etc_hosts: true\n"
        "users:\n"
        "  - name: student\n"
        "    gecos: Student User\n"
        "    sudo: ALL=(ALL) NOPASSWD:ALL\n"
        "    groups: users, admin\n"
        "    shell: /bin/bash\n"
        "    lock_passwd: false\n"
        "    plain_text_passwd: \"1234\"\n"
        "package_update: true\n"
        "package_upgrade: true\n"
        "packages:\n"
        "  - nginx\n"
        "write_files:\n"
        "  - path: /var/www/html/index.html\n"
        "    permissions: '0644'\n"
        "    owner: root:root\n"
        "    content: |\n"
        "      <html><body><h1>%s</h1></body></html>\n"
        "runcmd:\n"
        "  - systemctl enable nginx\n"
        "  - systemctl start nginx\n",
        vm_name, message);
    fclose(f);

    printf("Create meta-data\n");
    f = fopen("meta-data", "w");
    fprintf(f, "instance-id: %s\nlocal-hostname: %s\n", vm_name, vm_name);
    fclose(f);

    printf("Creating cloud-data.iso\n");
    snprintf(cmd, CMD_BUFFER, "genisoimage -output cloud-data.iso -volid cidata -rational-rock -joliet user-data meta-data");
    system(cmd);

    printf("Create VM\n");
    snprintf(cmd, CMD_BUFFER,
        "virt-install --name %s "
        "--ram %s --vcpus %s "
        "--disk path=./jammy-server-cloudimg-amd64.img,format=qcow2 "
        "--disk path=cloud-data.iso,device=cdrom "
        "--os-variant ubuntu22.04 "
        "--virt-type qemu "
        "--network network=vmnet1,model=virtio "
        "--import --noautoconsole",
        vm_name, ram, vcpus);
    system(cmd);

    printf("Get VM IP\n");
    snprintf(cmd, CMD_BUFFER, "virsh domifaddr %s", vm_name);
    system(cmd);

    printf("VM '%s' created\n", vm_name);

    return 0;
}
