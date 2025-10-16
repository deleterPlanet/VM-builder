#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMD_BUFFER 1024
#define IP_BUFFER 128

void get_vm_ip(char *vm_name, char *ip_buffer) {
    char cmd[CMD_BUFFER];
    FILE *fp;

    snprintf(cmd, sizeof(cmd), "virsh domifaddr %s | awk '/ipv4/ {print $4}' | cut -d'/' -f1", vm_name);

    while(1) {  // read ip every 5 sec
        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return;
        }
        if (fgets(ip_buffer, IP_BUFFER, fp) != NULL) {
            ip_buffer[strcspn(ip_buffer, "\n")] = 0;
            pclose(fp);
            if (strlen(ip_buffer) > 0)
                return;
        }
        pclose(fp);
        printf("Waiting for VM IP...\n");
        sleep(5);
    }
    strcpy(ip_buffer, "UNKNOWN");
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <vm_name> <ram_mb> <vcpus> <web_message>\n", argv[0]);
        return 1;
    }

    char *vm_name = argv[1];
    char *ram = argv[2];
    char *vcpus = argv[3];
    char *message = argv[4];
    char *virt_type = (argc == 5) ? argv[5] : "qemu";
    char cmd[CMD_BUFFER];
    char ip[IP_BUFFER] = "";

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
        "--virt-type %s "
        "--network network=default,model=virtio "
        "--import --noautoconsole",
        vm_name, ram, vcpus, virt_type);
    system(cmd);

    printf("Get VM IP\n");
    get_vm_ip(vm_name, ip);

    if (strcmp(ip, "UNKNOWN") != 0){
        printf("VM '%s' created.\n Website available at: http://%s\n", vm_name, ip);
    }else{
        printf("Could not determine IP address for VM '%s'\n", vm_name);
    }

    return 0;
}
