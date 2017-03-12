# Custom image available in my private docker registry :)
FROM ubuntuvivid

EXPOSE 8000

# See: https://blog.phusion.nl/2015/01/20/docker-and-the-pid-1-zombie-reaping-problem/
# I wonder if this could also work: http://veithen.github.io/2014/11/16/sigterm-propagation.html
ENV TINI_VERSION v0.10.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini-static /tini
RUN chmod +x /tini

ENTRYPOINT ["/tini", "--"]
CMD ["/code/bin/someprogram"]

COPY ./Dockerfile /Dockerfile
ADD ./build/* /code
