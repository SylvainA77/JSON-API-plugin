# Use Rocky Linux as the base image
FROM rockylinux/rockylinux:8 AS build

# Add a label for the image author
LABEL maintainer="arbaudie.it@gmail.com"

# Install necessary tools and libraries
RUN dnf update -y && yum install -y epel-release
RUN dnf install -y git gcc gcc-c++ cmake curl net-tools cjson-devel
RUN dnf --enablerepo=devel install -y libmicrohttpd-devel
#housekeeping time
RUN yum clean all


# Add MariaDB 11.4 repository and install it + devel headers + housekeeping
RUN curl -sSL https://downloads.mariadb.com/MariaDB/mariadb_repo_setup | bash -s -- --mariadb-server-version="mariadb-11.4" && \
    yum install -y MariaDB-server MariaDB-client MariaDB-devel && yum clean all

# Set the working directory
WORKDIR /opt

# Add :opt to the $PATH for easy calling from inside the container
ENV PATH="${PATH}:/opt"
# setting the branch name we want to clone
ENV GITBRANCH=GET

RUN printf "[mariadb]\nplugin-load-add=libjson2sql\nplugin_maturity=unknown\n" > /etc/load_jsonplugin.cnf
RUN printf "#!/bin/bash \n cd /app \n git clone -b $GITBRANCH --single-branch --depth 1 https://github.com/MariaDBJones/json2sql-plugin.git . \n mkdir build \n cd build \n cmake .. \n" > clone
RUN printf "#!/bin/bash \n cd /app/build \n cmake --build . --verbose \n" > build
RUN printf "#!/bin/bash \n cp -f /app/plugin/libjson2sql.so /usr/lib64/mysql/plugin/ \n mariadbd-safe --defaults-extra-file=/etc/load_jsonplugin.cnf &" > deploy
RUN printf "#!/bin/bash \n mariadb -e 'shutdown;' \n rm -f /usr/lib64/mysql/plugin/libjson2sql.so\n" > clean

WORKDIR /app

# Expose the default json2sql port
EXPOSE 3000

# Start command for the container (MariaDB will not auto-start)
CMD ["/bin/bash"]
