FROM debian:buster-slim

RUN apt-get update && apt-get install -y --no-install-recommends python3 wget ca-certificates && apt-get clean
WORKDIR /root
RUN wget https://registrationcenter-download.intel.com/akdlm/IRC_NAS/96aa5993-5b22-4a9b-91ab-da679f422594/intel-oneapi-base-toolkit-2025.0.0.885_offline.sh && sh ./intel-oneapi-base-toolkit-2025.0.0.885_offline.sh -a --silent --eula accept && rm ./intel-oneapi-base-toolkit-2025.0.0.885_offline.sh

CMD ["bash","measure.sh"]
