# Custom image available in my private docker registry :)
FROM ubuntuvivid

EXPOSE 8000

ARG artifact_name
ENV ARTIFACT_NAME=${artifact_name}

# See: https://blog.phusion.nl/2015/01/20/docker-and-the-pid-1-zombie-reaping-problem/
# I wonder if this could also work: http://veithen.github.io/2014/11/16/sigterm-propagation.html
#
ADD https://github.com/krallin/tini/releases/download/v0.14.0/tini-static /tini
# Alternative with tini already downloaded (in case there is not github access)
# COPY /PATH/TO/TINI/${TINI_VERSION}/tini-static /tini
RUN chmod +x /tini
ENTRYPOINT ["/tini", "--"]
CMD java -jar /code/${ARTIFACT_NAME}


RUN mkdir /code
COPY /code/* /code/
